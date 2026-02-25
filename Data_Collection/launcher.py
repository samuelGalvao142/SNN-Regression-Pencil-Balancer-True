# launcher.py
import subprocess
import time

def main():
    try:
        # Run both scripts in parallel 
        p2 = subprocess.Popen(["python", "Data_Collection/camera_reader.py"])
        time.sleep(2)
        p1 = subprocess.Popen(["python", "Data_Collection/srv02_control.py"])

        print("Both processes are running in parallel. CTRL+C to stop.")

        # Wait for both to finish
        p1.wait()
        p2.wait()

    except KeyboardInterrupt:
        print("Stopping processes...")
        p1.terminate()
        p2.terminate()

if __name__ == "__main__":
    main()
