import os
import pandas as pd
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score, classification_report, confusion_matrix
import joblib

def main():

    data_path = 'data/processed/features.csv'
    if not os.path.exists(data_path):
        print(f"ERROR: Could not find {data_path}")
        return

    df = pd.read_csv(data_path)

    train_df = df[df['session'] == 1]
    test_df = df[df['session'] == 6]

    if test_df.empty:
        print("Warning: No test data found...")
        return

    drop_cols = ['label', 'session']
    X_train = train_df.drop(columns=drop_cols)
    y_train = train_df['label']

    X_test = test_df.drop(columns = drop_cols)
    y_test = test_df['label']

    print(f"Training on {len(X_train)} windows... Testing on {len(X_test)} unseen windows.")

    print("Training Random Forest Classifier...")

    clf = RandomForestClassifier(n_estimators=20, max_depth=10, random_state=42)
    clf.fit(X_train, y_train)

    y_pred = clf.predict(X_test)
    acc = accuracy_score(y_test, y_pred)

    print("\n" + "="*50)
    print(f"MODEL ACCURACY ON UNSEEN DATA: {acc * 100: .2f}%")
    print("="*50)

    print("\nConfusion Matrix (Rows = Actual, Columns = Predicted): ")
    print(confusion_matrix(y_test, y_pred))

    print("\nDetailed Classification Report:")
    print(classification_report(y_test, y_pred, target_names=[
        "0: Normal", "1: Transit", "2: Tilt", "3: Opened", "4: Impact"
    ]))

    os.makedirs('models', exist_ok = True)
    model_path = 'models/senseweave_rf.pkl'
    joblib.dump(clf, model_path)
    print(f"\nSaved trained Python model to {model_path}")


if __name__ == '__main__':
    main()
