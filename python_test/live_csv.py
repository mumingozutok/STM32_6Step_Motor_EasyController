import serial
import csv
from datetime import datetime
import os

# Seri port ayarları
SERIAL_PORT = 'COM5'        # kendi portuna göre değiştir
BAUD_RATE = 921600

# Zaman damgası ile eşsiz dosya ismi oluştur
timestamp_str = datetime.now().strftime('%Y%m%d_%H%M%S')
CSV_FILENAME = f'rpm_log_{timestamp_str}.csv'

# Seri portu aç
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

# CSV dosyasını oluştur ve başlıkları yaz
with open(CSV_FILENAME, mode='w', newline='') as csv_file:
    csv_writer = csv.writer(csv_file)
    csv_writer.writerow([
        'Timestamp',
        'rpm_value',
        'pwm_duty',        
        'pid_output',
        'target_rpm'
    ])

    print(f"[{datetime.now()}] Kayıt başladı: {CSV_FILENAME} (Çıkmak için Ctrl+C)")

    try:
        while True:
            line = ser.readline().decode('utf-8').strip()

            if line:
                try:
                    parts = list(map(int, line.split(',')))
                    if len(parts) == 4:
                        rpm_value, pwm_duty, pid_output, target_rpm = parts
                        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
                        csv_writer.writerow([
                            timestamp,
                            rpm_value,
                            pwm_duty,                            
                            pid_output,
                            target_rpm,

                        ])
                        print(f"{timestamp} -> H:{rpm_value},  U:{pwm_duty}, W:{pid_output}, Iu:{target_rpm}")
                    else:
                        print(f"Format hatası (4 değil {len(parts)} değer): {line}")
                except ValueError:
                    print(f"Hatalı veri: {line}")

    except KeyboardInterrupt:
        print("\nKayıt durduruldu.")

ser.close()