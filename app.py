from flask import Flask, render_template, request, jsonify
import serial
import time

app = Flask(__name__)

# Настройки Serial
serial_port = 'COM3'  # Укажите порт, к которому подключена Arduino
baud_rate = 9600
arduino = None

try:
    arduino = serial.Serial(serial_port, baud_rate, timeout=1)
    time.sleep(2)  # Ожидание инициализации Serial
except Exception as e:
    print(f"Serial error: {e}")

# Состояния устройств
device_states = {
    "motorA1A2": "stop",  # up, down, stop
    "motor8_11": "stop",  # up, down, stop
    "servo2_3": "close",  # open, close
    "servo4_5": "close",  # open, close
    "led1": "off",        # on, off
    "led2": "off",        # on, off
    "relay1": "off",      # on, off
    "relay2": "off",      # on, off
    "relay3": "off",      # on, off
    "relay4": "off",      # on, off
    "relaych1": "off",    # on, off (четырехканальное реле)
    "relaych2": "off",    # on, off
    "relaych3": "off",    # on, off
    "relaych4": "off"     # on, off
}

# Данные о расходе воды
water_data = {
    "total_water": 0.0,  # Общий расход воды (литры)
    "flow_rate": 0.0     # Скорость потока (литры/минуту)
}

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/get_states', methods=['GET'])
def get_states():
    return jsonify(device_states)

@app.route('/get_water_flow', methods=['GET'])
def get_water_flow():
    return jsonify(water_data)

@app.route('/update_state', methods=['POST'])
def update_state():
    data = request.json
    device = data.get('device')
    state = data.get('state').lower()

    if device in device_states:
        device_states[device] = state
        if arduino and arduino.is_open:
            command = f"{device.upper()}_{state.upper()}\n"
            print(f"Sending command: {command}")  # Отладочное сообщение
            arduino.write(command.encode())
        return jsonify({"status": "success", "device": device, "state": state})
    else:
        return jsonify({"status": "error", "message": "Invalid device"}), 400

@app.route('/reset_water_counter', methods=['POST'])
def reset_water_counter():
    if arduino and arduino.is_open:
        arduino.write(b"RESET_WATER\n")
    water_data["total_water"] = 0.0
    return jsonify(water_data)

# Функция для обновления данных о расходе воды
def update_water_data():
    if arduino and arduino.is_open:
        arduino.write(b"GET_WATER_DATA\n")
        response = arduino.readline().decode().strip()
        if response:
            try:
                total_water, flow_rate = map(float, response.split(','))
                water_data["total_water"] = total_water
                water_data["flow_rate"] = flow_rate
                print(f"Water Data: Total = {total_water}, Flow Rate = {flow_rate}")
            except ValueError:
                print("Invalid water data format")

# Периодическое обновление данных о расходе воды
def water_data_updater():
    while True:
        update_water_data()
        time.sleep(1)

if __name__ == '__main__':
    import threading
    # Запуск потока для обновления данных о расходе воды
    threading.Thread(target=water_data_updater, daemon=True).start()
    # Запуск Flask-приложения
    app.run(host='0.0.0.0', port=5000)