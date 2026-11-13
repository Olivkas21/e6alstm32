import serial
import time

PORT = '/dev/ttyACM0'
BAUD = 9600
while(1 == 1):
    for i in range(0, 100, 10):
        duty_percent = i  # от 0 до 100

        packet = [0x7C, 0x02, duty_percent, 0x00, 0x00, 0x00, 0x01]
        xor_sum = 0
        for i in range(1, 7):
            xor_sum ^= packet[i]
        packet.append(xor_sum)

        print(f"Установка скважности {duty_percent}%")
        print(f"Пакет: {[hex(b) for b in packet]}")

        try:
            ser = serial.Serial(PORT, BAUD, timeout=1)
            time.sleep(0.5)
            ser.write(bytes(packet))
            time.sleep(0.2)
            response = ser.read(ser.in_waiting)
            if response:
                print(f"Ответ: {list(response)}")
            ser.close()
        except Exception as e:
            print(f"Ошибка: {e}")
