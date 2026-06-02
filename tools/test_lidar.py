import serial
import time

def test_c1_lidar():
    baud = 460800
    print(f"Testing RPLidar C1 at baudrate: {baud}")
    try:
        ser = serial.Serial('/dev/ttyUSB0', baud, timeout=1)
        ser.dtr = False # usually drops DTR to turn on motor or wake up
        time.sleep(1)
        
        # 1. Stop
        ser.write(b'\xA5\x25')
        time.sleep(0.1)
        ser.read_all()
        
        # 2. Reset
        ser.write(b'\xA5\x40')
        time.sleep(1)
        ser.read_all()
        
        # 3. Get Device Info
        ser.write(b'\xA5\x50')
        time.sleep(0.5)
        
        res = ser.read_all()
        if len(res) > 0:
            print(f"[SUCCESS] Got response from C1: {res.hex().upper()}")
            ser.close()
            return True
        else:
            print("[FAILED] No response. Make sure it's spinning and connected properly.")
            ser.close()
            return False
    except Exception as e:
        print(f"Error: {e}")
        return False

test_c1_lidar()
