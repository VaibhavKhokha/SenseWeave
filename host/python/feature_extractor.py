import os
import pandas as pd
import numpy as np
import glob

INPUT_DIR = 'data/raw'
OUTPUT_DIR = 'data/processed'
WINDOW_SIZE = 50
STEP_SIZE = 25

def extract_features(window, label, session):

    features = {}
    features['label'] = label
    features['session'] = session

    for axis in ['ax', 'ay', 'az']:
        features[f'{axis}_mean'] = window[axis].mean()
        features[f'{axis}_std'] = window[axis].std()
        features[f'{axis}_min'] = window[axis].min()
        features[f'{axis}_max'] = window[axis].max()

    for axis in ['gx', 'gy', 'gz']:
        features[f'{axis}_mean'] = window[axis].mean()
        features[f'{axis}_std'] = window[axis].std()
        features[f'{axis}_min'] = window[axis].min()
        features[f'{axis}_max'] = window[axis].max()


    features['ldr_mean'] = window['ldr'].mean()

    features['dist_mean'] = window['dist'].mean()

    features['vib_max'] = window['vib'].max()

    return features


def main():

    os.makedirs(OUTPUT_DIR, exist_ok = True)
    all_features = []

    file_pattern = os.path.join(INPUT_DIR, '*.csv')
    csv_files = glob.glob(file_pattern)


    if not csv_files:
        print(f"No csv files found in {INPUT_DIR}...")
        return

    print(f"Found {len(csv_files)} data files. Chopping into 1-sec window...")

    for file in csv_files:
        basename = os.path.basename(file)
        parts = basename.replace('.csv', '').split('_')
        label = int(parts[1])
        session = int(parts[3])

        df = pd.read_csv(file, skipinitialspace=True)
        df.columns = df.columns.str.strip()

        for start in range(0, len(df) - WINDOW_SIZE + 1, STEP_SIZE):
            window = df.iloc[start : start + WINDOW_SIZE]

            f = extract_features(window, label, session)
            all_features.append(f)

    features_df = pd.DataFrame(all_features)

    output_file = os.path.join(OUTPUT_DIR, 'features.csv')
    features_df.to_csv(output_file, index=False)

    print(f"\nSuccess...")
    print(f"Saved to: {output_file}...")

    print("\nFeature columns: ")
    print(list(features_df.columns))

if __name__ == '__main__':
    main()
