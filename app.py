from flask import Flask, render_template, request, jsonify
import serial
import time

app = Flask(__name__)

# Настройки Serial
serial_port = 'COM4'  # Укажите порт, к которому подключена Arduino
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
    "motor1": "off",      # on, off (насос Нижний бьеф)
    "motor2": "off",      # on, off (насос Верхний бьеф)
    "water_level_empty": "low",  # high, low
    "water_level_full": "low"    # high, low
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

# @app.route('/get_water_flow', methods=['GET'])
# def get_water_flow():
#     return jsonify(water_data)

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

@app.route('/update_motor_state', methods=['POST'])
def update_motor_state():
    data = request.json
    motor = data.get('motor')
    state = data.get('state').lower()

    if motor in ["motor1", "motor2"] and state in ["on", "off"]:
        device_states[motor] = state
        if arduino and arduino.is_open:
            command = f"{motor.upper()}_{state.upper()}\n"
            print(f"Sending command: {command}")  # Отладочное сообщение
            arduino.write(command.encode())
        return jsonify({"status": "success", "motor": motor, "state": state})
    else:
        return jsonify({"status": "error", "message": "Invalid motor or state"}), 400

# Функция для обновления данных
def update_data():
    if arduino and arduino.is_open:
        response = arduino.readline().decode().strip()
        try:
            data = list(map(str, response.split(',')))
            distanse_down = float(data[0])
            distanse_middle = float(data[1])
            distanse_up = float(data[2])
            total_water = float(data[3])
            flow_rate = float(data[4])
            waterLevelEmpty = data[5]
            waterLevelFull = data[6]
            print(distanse_down, distanse_middle, distanse_up, total_water, flow_rate, waterLevelEmpty, waterLevelFull)
        except ValueError:
            print("Invalid water data format:    ", response)

def data_updater():
    while True:
        # update_water_data()
        update_data()
        time.sleep(0.2)

if __name__ == '__main__':
    import threading
    # Запуск потока для обновления данных о расходе воды
    threading.Thread(target=data_updater, daemon=True).start()
    # Запуск Flask-приложения
    app.run(host='0.0.0.0', port=5000)