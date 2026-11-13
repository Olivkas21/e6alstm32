import serial

ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)

print("Нажми синюю кнопку на плате...")
while True:
    data = ser.read(384)
    if data:
        print(f"Принято {len(data)} байт.")
        print(f"Первые 15 чисел: {list(data[:15])}")
        print(f"Следующие 15 чисел: {list(data[128:143])}")