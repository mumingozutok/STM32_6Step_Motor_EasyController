import serial
import csv
from datetime import datetime
import os

# Seri port ayarları
SERIAL_PORT = 'COM14'        # kendi portuna göre değiştir
BAUD_RATE = 921600

# Zaman damgası ile eşsiz dosya ismi oluştur
timestamp_str = datetime.now().strftime('%Y%m%d_%H%M%S')
CSV_FILENAME = f'adc_log_{timestamp_str}.csv'

# Seri portu aç
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

# CSV dosyasını oluştur ve başlık satırını yaz
with open(CSV_FILENAME, mode='w', newline='') as csv_file:
    csv_writer = csv.writer(csv_file)
    csv_writer.writerow(['Timestamp', 'CommutationState', 'U', 'V', 'W'])

    print(f"[{datetime.now()}] Kayıt başladı: {CSV_FILENAME} (Çıkmak için Ctrl+C)")

    try:
        while True:
            line = ser.readline().decode('utf-8').strip()

            if line:
                try:
                    parts = list(map(int, line.split(',')))
                    if len(parts) == 4:
                        commutation_state, u, v, w = parts
                        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
                        csv_writer.writerow([timestamp, commutation_state, u, v, w])
                        print(f"{timestamp} -> State:{commutation_state}, U:{u}, V:{v}, W:{w}")
                    else:
                        print(f"Format hatası: {line}")
                except ValueError:
                    print(f"Hatalı veri: {line}")

    except KeyboardInterrupt:
        print("\nKayıt durduruldu.")

ser.close()