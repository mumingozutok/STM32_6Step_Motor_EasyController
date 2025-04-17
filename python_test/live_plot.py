import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.widgets import TextBox, Button
from collections import deque

# Seri port ayarları
ser = serial.Serial('COM26', 115200, timeout=1)

# Veri saklamak için dairesel buffer
max_len = 200
encoder_pos_data = deque([0]*max_len, maxlen=max_len)
comm_step_data = deque([0]*max_len, maxlen=max_len)
loop_state_data = deque([0]*max_len, maxlen=max_len)

# Şekil ve alt grafik oluştur
fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(10, 8))
fig.subplots_adjust(bottom=0.25)

# PWM textbox için konum
axbox = plt.axes([0.15, 0.05, 0.2, 0.075])
text_box = TextBox(axbox, 'PWM Duty', initial='25')

# PWM gönderme butonu
axbutton = plt.axes([0.4, 0.05, 0.1, 0.075])
button = Button(axbutton, 'Gönder')


def send_pwm(event):
    try:
        pwm_value = int(text_box.text)
        if 0 <= pwm_value <= 100:
            pwm_command = f'pwm:{pwm_value}\r\n'
            ser.write(pwm_command.encode())
            print(f"PWM gönderildi: {pwm_value}")
        else:
            print("PWM 0-100 arasında olmalı.")
    except ValueError:
        print("Geçersiz PWM değeri.")

button.on_clicked(send_pwm)

# Grafik çizim fonksiyonu
def animate(i):
    try:
        line = ser.readline().decode().strip()
        if line.startswith("pwm:"):
            return  # PWM cevabı ise çizim yapılmaz
        parts = line.split(',')
        if len(parts) == 3:
            encoder_pos = int(parts[0])
            comm_step = int(parts[1])
            loop_state = int(parts[2])

            encoder_pos_data.append(encoder_pos)
            comm_step_data.append(comm_step)
            loop_state_data.append(loop_state)

            ax1.clear()
            ax2.clear()
            ax3.clear()

            ax1.plot(encoder_pos_data, label="Encoder Pos")
            ax1.set_title("Encoder Position")
            ax1.grid(True)

            ax2.plot(comm_step_data, label="Comm Step", color='orange')
            ax2.set_title("Commutation Step")
            ax2.grid(True)

            ax3.plot(loop_state_data, label="Loop State", color='green')
            ax3.set_title("Loop State")
            ax3.grid(True)
    except Exception as e:
        print(f"Hata: {e}")

ani = animation.FuncAnimation(fig, animate, interval=100)
plt.tight_layout()
plt.show()
