import os
import pandas as pd
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score
from micromlgen import port

def main():
    data_path = 'data/processed/features.csv'
    out_path = 'firmware/esp32s3/include/model_optimized.h'
    
    if not os.path.exists(data_path):
        print(f"Error: Could not find {data_path}")
        return
        
    df = pd.read_csv(data_path)
    train_df = df[df['session'] == 1]
    test_df = df[df['session'] == 6]
    
    # We are officially pruning the noisy sensors from our architecture!
    drop_cols = ['label', 'session', 'dist_mean', 'vib_max']
    
    X_train = train_df.drop(columns=drop_cols, errors='ignore')
    y_train = train_df['label']
    
    X_test = test_df.drop(columns=drop_cols, errors='ignore')
    y_test = test_df['label']
    
    print("Training Optimized Random Forest (IMU + LDR Only)...")
    clf = RandomForestClassifier(n_estimators=20, max_depth=10, random_state=42)
    clf.fit(X_train, y_train)
    
    y_pred = clf.predict(X_test)
    acc = accuracy_score(y_test, y_pred)
    
    print("\n" + "="*50)
    print(f"OPTIMIZED MODEL ACCURACY ON UNSEEN DATA: {acc * 100:.2f}%")
    print("="*50 + "\n")
    
    print("Translating Optimized Model to C++...")
    # Generate C++ code with a new class name to avoid conflicts
    c_code = port(clf, classname='SenseWeaveOptimized')
    
    with open(out_path, 'w') as f:
        f.write(c_code)
        
    print(f"Success! Optimized C++ model saved to: {out_path}")

if __name__ == '__main__':
    main()