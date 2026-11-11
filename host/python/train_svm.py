import os
import pandas as pd
from sklearn.svm import SVC
from sklearn.metrics import accuracy_score
from micromlgen import port

def main():
    data_path = 'data/processed/features.csv'
    out_path = 'firmware/esp32s3/include/svm_model.h'
    
    if not os.path.exists(data_path):
        print(f"Error: Could not find {data_path}")
        return
        
    df = pd.read_csv(data_path)

    # 1. Strict Session-Based Train/Test Split
    train_df = df[df['session'] == 1]
    test_df = df[df['session'] == 6]
    
    drop_cols = ['label', 'session']
    X_train = train_df.drop(columns=drop_cols)
    y_train = train_df['label']

    X_test = test_df.drop(columns=drop_cols)
    y_test = test_df['label']

    print(f"Training on {len(X_train)} windows... Testing on {len(X_test)} unseen windows.")

    # 2. Train a Support Vector Machine (Linear Kernel)
    print("Training Support Vector Machine (SVM)...")
    clf = SVC(kernel='linear', gamma=0.001, random_state=42)
    clf.fit(X_train, y_train)

    # 3. Evaluate Accuracy
    y_pred = clf.predict(X_test)
    acc = accuracy_score(y_test, y_pred)
    
    print("\n" + "="*50)
    print(f"SVM MODEL ACCURACY ON UNSEEN DATA: {acc * 100:.2f}%")
    print("="*50 + "\n")
    
    # 4. Export directly to C++
    print("Translating SVM to C++...")
    c_code = port(clf, classname='SenseWeaveSVM')
    
    with open(out_path, 'w') as f:
        f.write(c_code)
        
    print(f"Success! Embedded SVM model saved to: {out_path}")

if __name__ == '__main__':
    main()