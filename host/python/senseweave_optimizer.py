import sys
import time
import re
import serial
import serial.tools.list_ports
import json
import os
from datetime import datetime

def find_esp32_port():
    """Automatically finds the COM port connected to the ESP32-S3."""
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if any(keyword in port.description for keyword in ["USB", "UART", "CH340", "CP210", "JTAG"]):
            return port.device
    return None

def main():
    print("\n" + "="*50)
    print(" SENSEWEAVE: HARDWARE-IN-THE-LOOP OPTIMIZER ")
    print("="*50)
    
    # 1. Prompt-Based Interface for Developer Constraints
    try:
        min_acc = float(input("Enter Minimum Required Accuracy (%)  [e.g., 70.0]: "))
        max_lat = float(input("Enter Maximum Allowed Latency (us) [e.g., 25.0]: "))
    except ValueError:
        print("Invalid input. Please enter numerical values.")
        return

    port = find_esp32_port()
    if not port:
        port = input("Error: Could not find ESP32 port. Enter manually (e.g., COM4): ")
        
    print(f"\n[HIL] Connecting to {port} to profile hardware...")
    
    # 2. Known offline metrics from our ML pipeline
    candidates = {
        "Baseline RF": {"accuracy": 69.35, "latency": None, "sensors": "IMU, LDR, Ultrasonic, Vibration"},
        "Optimized RF": {"accuracy": 70.85, "latency": None, "sensors": "IMU, LDR (Pruned)"}
    }

    # 3. Read live online metrics from the physical silicon
    try:
        ser = serial.Serial(port, 115200, timeout=10)
        
        # Give the serial connection 1 second to stabilize, then send the 'B' trigger
        time.sleep(1.0)
        ser.write(b'B')
        
        print("[HIL] Sent benchmark request. Waiting for device...\n")
        
        benchmark_done = False

        while not benchmark_done:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            
            if "[Baseline RF - 27 Feats]" in line:
                match = re.search(r"True Avg Latency: ([\d\.]+) us", line)
                if match:
                    candidates["Baseline RF"]["latency"] = float(match.group(1))
                    print(f"  -> Hardware reported Baseline Latency:  {candidates['Baseline RF']['latency']} us")
                    
            elif "[Optimized RF - 25 Feats]" in line:
                match = re.search(r"True Avg Latency: ([\d\.]+) us", line)
                if match:
                    candidates["Optimized RF"]["latency"] = float(match.group(1))
                    print(f"  -> Hardware reported Optimized Latency: {candidates['Optimized RF']['latency']} us")
                    benchmark_done = True
                    
        ser.close()
    except Exception as e:
        print(f"\nSerial Error: {e}")
        print("CRITICAL: Make sure the PlatformIO Serial Monitor is CLOSED before running this tool!")
        return

    # 4. Constraint-Aware Recommendation Engine
    print("\n" + "="*50)
    print(" EVALUATION RESULTS ")
    print("="*50)
    
    best_candidate = None
    
    for name, metrics in candidates.items():
        acc = metrics["accuracy"]
        lat = metrics["latency"]
        
        print(f"Candidate: {name}")
        print(f" - Sensors: {metrics['sensors']}")
        print(f" - Accuracy: {acc}%")
        print(f" - Latency:  {lat} us")
        
        if acc < min_acc:
            print(f" [REJECTED] Accuracy {acc}% is below minimum {min_acc}%")
        elif lat > max_lat:
            print(f" [REJECTED] Latency {lat} us exceeds maximum {max_lat} us")
        else:
            print(" [ACCEPTED] Meets all hardware and task constraints!")
            # Rank based on accuracy
            if best_candidate is None or acc > candidates[best_candidate]["accuracy"]:
                best_candidate = name
        print("-")

    print("="*50)
    if best_candidate:
        print(f" RECOMMENDATION: Deploy >> {best_candidate} <<")
    else:
        print(" RECOMMENDATION: No configuration meets the constraints.")
    print("="*50 + "\n")

    # 5. Experiment Lifecycle Management (Logging)
    os.makedirs('experiments/results', exist_ok=True)
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    
    experiment_record = {
        "timestamp": timestamp,
        "constraints": {
            "min_accuracy_percent": min_acc,
            "max_latency_us": max_lat
        },
        "candidates_evaluated": candidates,
        "final_recommendation": best_candidate
    }
    
    log_filename = f"experiments/results/run_{timestamp}.json"
    with open(log_filename, 'w') as f:
        json.dump(experiment_record, f, indent=4)
        
    print(f"[LIFECYCLE] Experiment history securely saved to: {log_filename}\n")

if __name__ == '__main__':
    main()