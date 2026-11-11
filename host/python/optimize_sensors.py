import os
import pandas as pd
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score

def evaluate_subset(train_df, test_df, drop_features, config_name):
    """Trains an RF model and returns the accuracy after dropping specific features."""
    # Base columns to drop (the metadata)
    drop_cols = ['label', 'session'] + drop_features
    
    X_train = train_df.drop(columns=drop_cols, errors='ignore')
    y_train = train_df['label']
    
    X_test = test_df.drop(columns=drop_cols, errors='ignore')
    y_test = test_df['label']
    
    clf = RandomForestClassifier(n_estimators=20, max_depth=10, random_state=42)
    clf.fit(X_train, y_train)
    
    y_pred = clf.predict(X_test)
    acc = accuracy_score(y_test, y_pred)
    
    return acc * 100

def main():
    data_path = 'data/processed/features.csv'
    if not os.path.exists(data_path):
        print(f"Error: Could not find {data_path}")
        return
        
    df = pd.read_csv(data_path)
    train_df = df[df['session'] == 1]
    test_df = df[df['session'] == 6]

    # Define our sensor feature groups
    gyro_features = [f'{ax}_{stat}' for ax in ['gx','gy','gz'] for stat in ['mean','std','min','max']]
    accel_features = [f'{ax}_{stat}' for ax in ['ax','ay','az'] for stat in ['mean','std','min','max']]
    
    configurations = [
        {"name": "1. ALL SENSORS (Baseline)", "drop": []},
        {"name": "2. NO Ultrasonic", "drop": ['dist_mean']},
        {"name": "3. NO Vibration Switch", "drop": ['vib_max']},
        {"name": "4. NO Gyroscope", "drop": gyro_features},
        {"name": "5. NO LDR (Light)", "drop": ['ldr_mean']},
        {"name": "6. ACCEL + LDR ONLY", "drop": ['dist_mean', 'vib_max'] + gyro_features},
    ]
    
    print("==================================================")
    print("SENSEWEAVE: SENSOR-SUBSET OPTIMIZATION")
    print("==================================================")
    print(f"{'Configuration':<30} | {'Accuracy':<10}")
    print("-" * 45)
    
    for config in configurations:
        acc = evaluate_subset(train_df, test_df, config["drop"], config["name"])
        print(f"{config['name']:<30} | {acc:>6.2f}%")
        
    print("==================================================")

if __name__ == '__main__':
    main()