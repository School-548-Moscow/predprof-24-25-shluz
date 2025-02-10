import serial

try:
    arduino = serial.Serial('COM3', 9600, timeout=1)
    print("Успешно подключено!")
    arduino.close()
except serial.SerialException as e:
    print(f"Ошибка: {e}")