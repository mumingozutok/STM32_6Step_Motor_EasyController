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

# CSV dosyasını oluştur ve başlıkları yaz
with open(CSV_FILENAME, mode='w', newline='') as csv_file:
    csv_writer = csv.writer(csv_file)
    csv_writer.writerow([
        'Timestamp',
        'HallState',
        'CommutationState',
        'ADC_U',
        'ADC_V',
        'ADC_W',
        'current_line'
    ])

    print(f"[{datetime.now()}] Kayıt başladı: {CSV_FILENAME} (Çıkmak için Ctrl+C)")

    try:
        while True:
            line = ser.readline().decode('utf-8').strip()

            if line:
                try:
                    parts = list(map(int, line.split(',')))
                    if len(parts) == 6:
                        hall_state, comm_state, adc_u, adc_v, adc_w, current_line = parts
                        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
                        csv_writer.writerow([
                            timestamp,
                            hall_state,
                            comm_state,
                            adc_u,
                            adc_v,
                            adc_w,
                            current_line,

                        ])
                        print(f"{timestamp} -> H:{hall_state}, C:{comm_state}, U:{adc_u}, V:{adc_v}, W:{adc_w}, Iu:{current_line}")
                    else:
                        print(f"Format hatası (8 değil {len(parts)} değer): {line}")
                except ValueError:
                    print(f"Hatalı veri: {line}")

    except KeyboardInterrupt:
        print("\nKayıt durduruldu.")

ser.close()