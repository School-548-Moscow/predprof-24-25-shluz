#include <Servo.h>

// Параметры для сервоприводов
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;

// Параметры для драйверов двигателей L9110S
int motor1Pin1 = 8;
int motor1Pin2 = 9;
int motor2Pin1 = 10;
int motor2Pin2 = 11;
int motor3Pin1 = A1;
int motor3Pin2 = A2;
int motor4Pin1 = A3;
int motor4Pin2 = A4;

// Датчик расхода воды
int flowSensorPin = A0; // Используем пин A0 (цифровой пин 14) для датчика расхода воды
volatile uint16_t pulseCount = 0; // Счетчик импульсов
float flowRate = 0.0; // Скорость потока воды (л/с)
float totalWater = 0.0; // Общий объем израсходованной воды (л)
unsigned long lastMeasurement = 0; // Время последнего измерения

// Светодиоды
int led1RedPin = A8;
int led1GreenPin = A9;
int led2RedPin = A10;
int led2GreenPin = A11;

// Реле
int relay1Pin = 22;
int relay2Pin = 24;
int relay3Pin = 26;
int relay4Pin = 28;

// Четырехканальное реле (подключено к пинам 30, 32, 34, 36)
int relayChannel1Pin = 30;
int relayChannel2Pin = 32;
int relayChannel3Pin = 34;
int relayChannel4Pin = 36;

// Состояния устройств
int motorA1A2State = 0; // 1 - верх, 2 - низ, 0 - стоп
int motor8_11State = 0; // 1 - верх, 2 - низ, 0 - стоп
bool servo2_3State = false;  // false - закрыто, true - открыто
bool servo4_5State = false;  // false - закрыто, true - открыто
bool led1State = false;      // false - выключен, true - включен
bool led2State = false;      // false - выключен, true - включен
bool relay1State = true;    // false - выключено, true - включено
bool relay2State = true;    // false - выключено, true - включено
bool relay3State = true;    // false - выключено, true - включено
bool relay4State = true;    // false - выключено, true - включено

// Состояния четырехканального реле
bool relayChannel1State = false; // false - выключено, true - включено
bool relayChannel2State = false; // false - выключено, true - включено
bool relayChannel3State = false; // false - выключено, true - включено
bool relayChannel4State = false; // false - выключено, true - включено

// Функция для обработки прерываний от датчика расхода воды
void pulseCounter() {
  pulseCount++;
}

// Функция для управления сервоприводами
void controlServos(bool open2_3, bool open4_5) {
  if (open2_3) {
    servo1.write(-5); // Открыто
    servo2.write(95);
  } else {
    servo2.write(-5);
    delay(100);
    servo1.write(95); // Закрыто
  }

  if (open4_5) {
    servo3.write(-10); // Открыто
    servo4.write(95);
  } else {
    servo3.write(95); // Закрыто
    servo4.write(-10);
  }
}

// Функция для управления моторами
void controlMotors(int motorA1A2Up, int motor8_11Up) {
  if (motorA1A2Up == 1) {
    analogWrite(motor3Pin1, 255); // Верх
    analogWrite(motor3Pin2, 0);
    analogWrite(motor4Pin1, 255);
    analogWrite(motor4Pin2, 0);
  } else if (motorA1A2Up == 2) {
    analogWrite(motor3Pin1, 0); // вниз
    analogWrite(motor3Pin2, 255);
    analogWrite(motor4Pin1, 0);
    analogWrite(motor4Pin2, 255);
  } else if (motorA1A2Up == 0) {
    analogWrite(motor3Pin1, 0); // стоп
    analogWrite(motor3Pin2, 0);
    analogWrite(motor4Pin1, 0);
    analogWrite(motor4Pin2, 0);
  }

  if (motor8_11Up == 1) {
    analogWrite(motor1Pin1, 255); // Верх
    analogWrite(motor1Pin2, 0);
    analogWrite(motor2Pin1, 255);
    analogWrite(motor2Pin2, 0);
  } else if (motor8_11Up == 2) {
    analogWrite(motor1Pin1, 0); // вниз
    analogWrite(motor1Pin2, 255);
    analogWrite(motor2Pin1, 0);
    analogWrite(motor2Pin2, 255);
  } else if (motor8_11Up == 0) {
    analogWrite(motor1Pin1, 0); // стоп
    analogWrite(motor1Pin2, 0);
    analogWrite(motor2Pin1, 0);
    analogWrite(motor2Pin2, 0);
  }
}

// Функция для управления светодиодами
void controlLEDs(bool led1On, bool led2On) {
  digitalWrite(led1RedPin, led1On ? HIGH : LOW);
  digitalWrite(led1GreenPin, led1On ? LOW : HIGH);
  digitalWrite(led2RedPin, led2On ? HIGH : LOW);
  digitalWrite(led2GreenPin, led2On ? LOW : HIGH);
}

// Функция для управления реле
void controlRelays() {
  digitalWrite(relay1Pin, relay1State ? HIGH : LOW);
  digitalWrite(relay2Pin, relay2State ? HIGH : LOW);
  digitalWrite(relay3Pin, relay3State ? HIGH : LOW);
  digitalWrite(relay4Pin, relay4State ? HIGH : LOW);
}

// Функция для управления четырехканальным реле
void controlRelayChannels() {
  digitalWrite(relayChannel1Pin, relayChannel1State ? HIGH : LOW);
  digitalWrite(relayChannel2Pin, relayChannel2State ? HIGH : LOW);
  digitalWrite(relayChannel3Pin, relayChannel3State ? HIGH : LOW);
  digitalWrite(relayChannel4Pin, relayChannel4State ? HIGH : LOW);
}

void setup() {
  // Инициализация сервоприводов
  servo1.attach(2);
  servo2.attach(3);
  servo3.attach(4);
  servo4.attach(5);

  // Инициализация моторов
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(motor3Pin1, OUTPUT);
  pinMode(motor3Pin2, OUTPUT);
  pinMode(motor4Pin1, OUTPUT);
  pinMode(motor4Pin2, OUTPUT);

  // Инициализация светодиодов
  pinMode(led1RedPin, OUTPUT);
  pinMode(led1GreenPin, OUTPUT);
  pinMode(led2RedPin, OUTPUT);
  pinMode(led2GreenPin, OUTPUT);

  // Инициализация реле
  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);
  pinMode(relay3Pin, OUTPUT);
  pinMode(relay4Pin, OUTPUT);

  // Инициализация четырехканального реле
  pinMode(relayChannel1Pin, OUTPUT);
  pinMode(relayChannel2Pin, OUTPUT);
  pinMode(relayChannel3Pin, OUTPUT);
  pinMode(relayChannel4Pin, OUTPUT);

  // Инициализация датчика расхода воды
  pinMode(flowSensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, RISING);

  // Инициализация Serial
  Serial.begin(9600);
  Serial.println("System started"); // Отладочное сообщение
}

void loop() {
  // Обработка команд от Serial
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    Serial.println("Received command: " + command);  // Отладочное сообщение

    // Обработка команды запроса данных
    if (command == "GET_WATER_DATA") {
      // Отправляем данные сразу при получении запроса
      String data = String(totalWater) + "," + String(flowRate);
      Serial.println(data);
    }
    // Управление моторами A1,A2 и A3,A4
    else if (command == "MOTORA1A2_UP") {
      motorA1A2State = 1;
      Serial.println("MOTORA1A2: UP");
    } else if (command == "MOTORA1A2_DOWN") {
      motorA1A2State = 2;
      Serial.println("MOTORA1A2: DOWN");
    } else if (command == "MOTORA1A2_STOP") {
      motorA1A2State = 0;
      Serial.println("MOTORA1A2: STOP");
    }

    // Управление моторами 8,9 и 10,11
    else if (command == "MOTOR8_11_UP") {
      motor8_11State = 1;
      Serial.println("MOTOR8_11: UP");
    } else if (command == "MOTOR8_11_DOWN") {
      motor8_11State = 2;
      Serial.println("MOTOR8_11: DOWN");
    } else if (command == "MOTOR8_11_STOP") {
      motor8_11State = 0;
      Serial.println("MOTOR8_11: STOP");
    }

    // Управление сервоприводами 2,3
    else if (command == "SERVO2_3_OPEN") {
      servo2_3State = true;
      Serial.println("SERVO2_3: OPEN");
    } else if (command == "SERVO2_3_CLOSE") {
      servo2_3State = false;
      Serial.println("SERVO2_3: CLOSE");
    }

    // Управление сервоприводами 4,5
    else if (command == "SERVO4_5_OPEN") {
      servo4_5State = true;
      Serial.println("SERVO4_5: OPEN");
    } else if (command == "SERVO4_5_CLOSE") {
      servo4_5State = false;
      Serial.println("SERVO4_5: CLOSE");
    }

    // Управление светодиодами
    else if (command == "LED1_ON") {
      led1State = true;
      Serial.println("LED1: ON");
    } else if (command == "LED1_OFF") {
      led1State = false;
      Serial.println("LED1: OFF");
    } else if (command == "LED2_ON") {
      led2State = true;
      Serial.println("LED2: ON");
    } else if (command == "LED2_OFF") {
      led2State = false;
      Serial.println("LED2: OFF");
    }

    // Управление реле
    else if (command == "RELAY1_TOGGLE") {
      relay1State = !relay1State;
      Serial.println(relay1State ? "RELAY1: ON" : "RELAY1: OFF");
    } else if (command == "RELAY2_TOGGLE") {
      relay2State = !relay2State;
      Serial.println(relay2State ? "RELAY2: ON" : "RELAY2: OFF");
    } else if (command == "RELAY3_TOGGLE") {
      relay3State = !relay3State;
      Serial.println(relay3State ? "RELAY3: ON" : "RELAY3: OFF");
    } else if (command == "RELAY4_TOGGLE") {
      relay4State = !relay4State;
      Serial.println(relay4State ? "RELAY4: ON" : "RELAY4: OFF");
    }

    // Управление четырехканальным реле
    else if (command == "RELAYCH1_TOGGLE") {
      relayChannel1State = !relayChannel1State;
      Serial.println(relayChannel1State ? "RELAYCH1: ON" : "RELAYCH1: OFF");
    } else if (command == "RELAYCH2_TOGGLE") {
      relayChannel2State = !relayChannel2State;
      Serial.println(relayChannel2State ? "RELAYCH2: ON" : "RELAYCH2: OFF");
    } else if (command == "RELAYCH3_TOGGLE") {
      relayChannel3State = !relayChannel3State;
      Serial.println(relayChannel3State ? "RELAYCH3: ON" : "RELAYCH3: OFF");
    } else if (command == "RELAYCH4_TOGGLE") {
      relayChannel4State = !relayChannel4State;
      Serial.println(relayChannel4State ? "RELAYCH4: ON" : "RELAYCH4: OFF");
    }
  }

  // Управление устройствами
  controlServos(servo2_3State, servo4_5State);
  controlMotors(motorA1A2State, motor8_11State);
  controlLEDs(led1State, led2State);
  controlRelays();
  controlRelayChannels();

  // Обновление данных датчика каждую секунду
  if (millis() - lastMeasurement >= 1000) {
    detachInterrupt(digitalPinToInterrupt(flowSensorPin)); // Отключаем прерывание для безопасного доступа к переменной pulseCount
    float frequency = pulseCount; // Частота импульсов за 1 секунду
    pulseCount = 0; // Сбрасываем счетчик импульсов
    attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, RISING); // Включаем прерывание обратно

    // Расчет скорости потока воды (л/с) по формуле Q = F / (5.9F + 4570)
    flowRate = frequency / (5.9 * frequency + 4570);

    // Расчет общего объема израсходованной воды (л)
    totalWater += flowRate;

    // Сохраняем время последнего измерения
    lastMeasurement = millis();

    // Вывод данных в Serial для отладки
    Serial.print("Flow Rate: ");
    Serial.print(flowRate * 60, 2); // Переводим в л/мин
    Serial.print(" л/мин, Total Water: ");
    Serial.print(totalWater, 2);
    Serial.println(" л");
  }

  delay(100); // Задержка для стабильности
}