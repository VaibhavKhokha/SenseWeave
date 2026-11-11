import argparse
import yaml
import sys
import os
import subprocess
import serial
import serial.tools.list_ports
import time
import json
from datetime import datetime
import pandas as pd
from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import RandomForestClassifier
from sklearn.naive_bayes import GaussianNB
from sklearn.metrics import accuracy_score
from micromlgen import port

def load_config(config_path):
    try:
        with open(config_path, 'r') as file:
            return yaml.safe_load(file)
    except Exception as e:
        print(f"Error loading config: {e}")
        sys.exit(1)

def find_esp32_port():
    # search for connected esp32 ports automatically
    for p in serial.tools.list_ports.comports():
        if any(kw in p.description for kw in ["USB", "UART", "CH340", "CP210", "JTAG"]):
            return p.device
    return None

def instantiate_model(model_cfg):
    arch = model_cfg['architecture']
    if arch == "DecisionTree":
        return DecisionTreeClassifier(random_state=42, max_depth=10)
    elif arch == "RandomForest":
        return RandomForestClassifier(n_estimators=model_cfg.get('n_estimators', 20), max_depth=10, random_state=42)
    elif arch == "GaussianNB":
        return GaussianNB()
    
    print(f"Unsupported architecture: {arch}")
    sys.exit(1)

def generate_cpp_artifacts(clf, candidate_id, X_test, y_test):
    # export the model to c++
    c_code = port(clf, classname='SenseWeaveModel')
    with open('firmware/esp32s3/include/model.h', 'w') as f:
        f.write(c_code)
        
    num_features = X_test.shape[1]
    
    # find how many unique classes exist to set array sizes
    unique_classes = sorted(y_test.unique())
    num_vectors = len(unique_classes)
    
    # write the header file with macros and test vectors
    config_h = f"""#pragma once\n#define MODEL_NAME "{candidate_id}"\n#define NUM_FEATURES {num_features}\n#define NUM_TEST_VECTORS {num_vectors}\nnamespace senseweave {{\n    const float kTestVectors[NUM_TEST_VECTORS][NUM_FEATURES] = {{\n"""
    
    for target_class in unique_classes:
        class_X = X_test[y_test == target_class]
        if not class_X.empty:
            vec_str = ", ".join([f"{val:.6f}f" for val in class_X.iloc[0].values])
            config_h += f"        {{ {vec_str} }},\n"
        else:
            config_h += f"        {{ 0 }},\n"
            
    config_h += "    };\n}\n"
    with open('firmware/esp32s3/include/model_config.h', 'w') as f:
        f.write(config_h)

def execute_hil_profiling(port_name):
    # communicate with esp32 over serial to get benchmark data
    try:
        ser = serial.Serial(port_name, 115200, timeout=5)
        time.sleep(1.5)  
        ser.write(b'B')  
        
        metrics = {"latency_us": None, "ram_bytes": None}
        start_time = time.time()
        
        while time.time() - start_time < 5:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line.startswith("LATENCY_US:"):
                metrics["latency_us"] = float(line.split(":")[1])
            elif line.startswith("RAM_BYTES:"):
                metrics["ram_bytes"] = int(line.split(":")[1])
            elif line == "---HIL_BENCHMARK_END---":
                break
                
        ser.close()
        return metrics
    except Exception as e:
        print(f"Serial error: {e}")
        return None

def run_optimizer(config):
    print("\n--- SenseWeave HIL Optimizer ---")
    print(f"Task: {config['experiment_name']}")
    print(f"Target: {config['hardware_target']}\n")
    
    com_port = find_esp32_port()
    if not com_port:
        com_port = input("ESP32 port not found automatically. Enter manually (e.g., COM4): ")

    pio_path = os.path.expanduser(r"~\.platformio\penv\Scripts\pio.exe") if os.name == 'nt' else os.path.expanduser("~/.platformio/penv/bin/pio")

    # load dataset
    df = pd.read_csv(config['dataset']['path'])
    train_df = df[df['session'] == config['dataset']['train_session']]
    test_df = df[df['session'] == config['dataset']['test_session']]
    
    candidate_registry = []
    
    for subset in config['sensor_subsets']:
        drop_cols = ['label', 'session'] + subset.get('drop_features', [])
        X_train, y_train = train_df.drop(columns=drop_cols, errors='ignore'), train_df['label']
        X_test, y_test = test_df.drop(columns=drop_cols, errors='ignore'), test_df['label']
        
        for model_cfg in config['models_to_evaluate']:
            candidate_id = f"{model_cfg['name']}_{subset['name']}"
            print(f"Evaluating: {candidate_id}")
            
            # train model
            clf = instantiate_model(model_cfg)
            clf.fit(X_train, y_train)
            
            # patch for micromlgen compatibility with newer sklearn versions
            if isinstance(clf, GaussianNB) and not hasattr(clf, 'sigma_'):
                clf.sigma_ = clf.var_
                
            acc = accuracy_score(y_test, clf.predict(X_test)) * 100.0
            print(f"  Offline Accuracy: {acc:.2f}% (Features: {X_train.shape[1]})")
            
            # generate firmware and flash
            generate_cpp_artifacts(clf, candidate_id, X_test, y_test)
            
            print(f"  Compiling and flashing firmware...")
            try:
                subprocess.run([pio_path, "run", "-d", "firmware/esp32s3", "--target", "upload"], 
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
            except subprocess.CalledProcessError:
                print("  [Error] Flash failed. Check COM port connection.")
                continue
                
            # run hardware profiling
            print(f"  Profiling on hardware...")
            hil_metrics = execute_hil_profiling(com_port)
            
            if hil_metrics and hil_metrics['latency_us']:
                print(f"  HIL Result: {hil_metrics['latency_us']} us | {hil_metrics['ram_bytes']} Bytes Free RAM\n")
                candidate_registry.append({
                    "id": candidate_id, "accuracy": acc, "features": X_train.shape[1],
                    "latency_us": hil_metrics["latency_us"], "ram_bytes": hil_metrics["ram_bytes"]
                })

    # evaluate constraints
    print("-" * 50)
    print("FINAL RECOMMENDATION MATRIX")
    print("-" * 50)
    
    min_acc = config['constraints']['min_accuracy_percent']
    max_lat = config['constraints']['max_latency_us']
    best_candidate = None
    
    for c in candidate_registry:
        status = "REJECTED"
        if c['accuracy'] < min_acc:
            reason = f"Accuracy {c['accuracy']:.2f}% < {min_acc}%"
        elif c['latency_us'] > max_lat:
            reason = f"Latency {c['latency_us']}us > {max_lat}us"
        else:
            status = "ACCEPTED"
            reason = "Meets constraints"
            if best_candidate is None or c['accuracy'] > best_candidate['accuracy']:
                best_candidate = c
                
        print(f"[{status}] {c['id']}")
        print(f"  Metrics: {c['accuracy']:.2f}% Acc | {c['latency_us']:.2f} us")
        print(f"  Reason:  {reason}\n")
        
    print("-" * 50)
    if best_candidate:
        print(f"RECOMMENDED DEPLOYMENT: {best_candidate['id']}")
    else:
        print("No configuration met the required constraints.")
    print("-" * 50)
    
    # save results
    os.makedirs('experiments/results', exist_ok=True)
    ts = datetime.now().strftime("%Y%m%d_%H%M%S")
    log_file = f"experiments/results/run_{ts}.json"
    with open(log_file, 'w') as f:
        json.dump({"timestamp": ts, "config": config, "candidates": candidate_registry, "best": best_candidate}, f, indent=4)

def main():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command")
    opt_parser = subparsers.add_parser("optimize")
    opt_parser.add_argument("--config", required=True)
    
    args = parser.parse_args()
    if args.command == "optimize":
        run_optimizer(load_config(args.config))
    else:
        parser.print_help()

if __name__ == "__main__":
    main()