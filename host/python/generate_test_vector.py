import os
import pandas as pd
import joblib

FEATURES_PATH = 'data/processed/features.csv'
MODEL_PATH = 'models/senseweave_rf.pkl'

def main():
    if not os.path.exists(FEATURES_PATH) or not os.path.exists(MODEL_PATH):
        print("ERROR: Missing features.csv or senseweave_rf.pkl")
        return

    df = pd.read_csv(FEATURES_PATH)
    clf = joblib.load(MODEL_PATH)

    test_df = df[df['session'] == 6]
    feature_cols = [c for c in df.columns if c not in ['label', 'session']]

    print("Golden Test Vector from Session 6 --->")
    print(f"Total Features per vector: {len(feature_cols)}\n")

    for target_class in range(5):
        class_rows = test_df[test_df['label'] == target_class]
        if class_rows.empty:
            continue

        sample_row = class_rows.iloc[0]
        X_sample = sample_row[feature_cols].values.reshape(1, -1)

        py_pred = int(clf.predict(X_sample)[0])
        actual_label = int(sample_row['label'])

        cpp_values = ", ".join([f"{val:.6f}f" for val in X_sample[0]])
        print(f"Class {actual_label} Sample | Python Predicts: {py_pred}")
        print(f"const float kTestVector_Class{actual_label}[27] == {{ {cpp_values} }};\n")


if __name__ == '__main__':
    main()