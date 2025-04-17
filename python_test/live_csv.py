import serial
import csv
import time

# Port ayarları
port = "COM26"
baudrate = 115200
filename = f"enc_log_{int(time.time())}.csv"

# Seriyi başlat
ser = serial.Serial(port, baudrate, timeout=1)

# CSV dosyası oluştur
with open(filename, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(["Timestamp", "Encoder Position", "Commutation Step", "Loop State"])  # Başlık

    print("Veri kaydediliyor... Ctrl+C ile çıkabilirsiniz.")
    try:
        while True:
            line = ser.readline().decode('utf-8').strip()
            if line:
                parts = line.split(',')
                if len(parts) == 3:
                    timestamp = time.time()
                    writer.writerow([timestamp] + parts)
                    print(f"{timestamp:.2f} | {parts}")
    except KeyboardInterrupt:
        print("\nKayıt tamamlandı. Dosya:", filename)
        ser.close()
