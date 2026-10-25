import os
import joblib
from micromlgen import port

def main():
    model_path = 'models/senseweave_rf.pkl'
    out_path = 'firmware/esp32s3/include/model.h'

    if not os.path.exists(model_path):
        print(f"ERROR: Could not find {model_path}...")
        return

    print(f"Loading trained model from {model_path}...")
    clf = joblib.load(model_path)

    print("Translating Random Forest to pure C++...")
    c_code = port(clf, classname='SenseWeaveModel')

    with open(out_path, 'w') as f:
        f.write(c_code)

    print(f"Success... C++ Model saved to: {out_path}")

if __name__ == '__main__':
    main()
