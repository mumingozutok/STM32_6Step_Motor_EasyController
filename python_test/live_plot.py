import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque

# === Ayarlar ===
PORT = 'COM26'        # STM32'nin bağlı olduğu COM portunu buraya yaz
BAUD = 115200
MAX_POINTS = 500     # Grafikte kaç veri noktası tutulsun?

# === UART başlat ===
ser = serial.Serial(PORT, BAUD, timeout=1)

# === Grafik verileri ===
positions = deque(maxlen=MAX_POINTS)
steps = deque(maxlen=MAX_POINTS)
states = deque(maxlen=MAX_POINTS)
samples = deque(maxlen=MAX_POINTS)

# === Başlangıç değerleri ===
for _ in range(MAX_POINTS):
    positions.append(0)
    steps.append(0)
    states.append(0)
    samples.append(0)

# === Matplotlib hazırlığı ===
fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(8, 6))
fig.tight_layout(pad=2)

def update(frame):
    try:
        line = ser.readline().decode('utf-8').strip()
        if line:
            parts = line.split(',')
            if len(parts) == 3:
                pos = int(parts[0])
                step = int(parts[1])
                state = int(parts[2])

                positions.append(pos)
                steps.append(step)
                states.append(state)
                samples.append(samples[-1] + 1)

                # Güncelleme
                ax1.clear()
                ax2.clear()
                ax3.clear()

                ax1.plot(samples, positions, label="Encoder Pos")
                ax2.plot(samples, steps, label="Commutation Step", color='orange')
                ax3.plot(samples, states, label="Loop State", color='green')

                ax1.set_ylabel("Encoder")
                ax2.set_ylabel("Step")
                ax3.set_ylabel("State")
                ax3.set_xlabel("Sample")

                ax1.legend()
                ax2.legend()
                ax3.legend()

                ax1.grid(True)
                ax2.grid(True)
                ax3.grid(True)

    except Exception as e:
        print("Hata:", e)

ani = animation.FuncAnimation(fig, update, interval=50)
plt.show()
