import serial
import time
import os

PORT = 'COM4'
BAUD = 115200
OUTPUT_DIR = 'data/raw'

def main():
    os.makedirs(OUTPUT_DIR, exist_ok = True)

    print(f"Connecting to ESP32-S3 on {PORT}...")

    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
    except Exception as e:
        print(f"Failed to connect: {e}")
        print("ERROR: Make sure the Serial Monitor is CLOSED!")
        return

    time.sleep(2)

    while True:
        print("\n" + "="*50)
        label = input("Enter class label (0-4) to record, or 'q' to quit: ").strip()

        if label.lower() == 'q':
            break
        if label not in ['0', '1', '2', '3', '4']:
            print("Invalid label. Please enter 0, 1, 2, 3 or 4...")
            continue

        session = input("Enter session ID (e.g; 1 for train, 6 for test): ").strip()

        filename = f"class_{label}_session_{session}.csv"
        filepath = os.path.join(OUTPUT_DIR, filename)

        print(f"\nGet ready! Recording Class {label} to {filepath} in 3 seconds...")
        time.sleep(1)
        print("2...")
        time.sleep(1)
        print("1...")
        time.sleep(1)

        ser.reset_input_buffer()

        ser.write(label.encode())

        print(">>> RECORDING! <<<")

        write_header = not os.path.exists(filepath)

        try:
            with open(filepath, 'a', encoding='utf-8') as f:
                if write_header:
                    f.write("timestamp_ms, label, ax, ay, az, gx, gy, gz, ldr, dist, vib\n")

                while True:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        print(line)

                        if line[0].isdigit():
                            f.write(line + '\n')

        except KeyboardInterrupt:
            ser.write(b's')
            print(f"\n[STOPPED] Saved data to {filepath}")


    ser.close()
    print("Data Collection complete...")

if __name__ == "__main__":
    main()
