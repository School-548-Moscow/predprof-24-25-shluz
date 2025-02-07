#include <Adafruit_GFX.h>    // Библиотека для графики
#include <Adafruit_ST7735.h> // Библиотека для TFT-дисплея
#include <Servo.h>
#include <Wire.h>

// Параметры для TFT-дисплея
#define TFT_CS   53  // Используем пин 53 для CS
#define TFT_RST  6   // Пин для сброса (заменили на 6)
#define TFT_DC   7   // Пин для выбора данных/команд (заменили на 7)
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);


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
int flowSensorPin = A0;
volatile long pulseCount = 0;
float flowRate = 0.0;
float totalWater = 0.0;

// Светодиоды
int led1RedPin = A8;
int led1GreenPin = A9;
int led2RedPin = A10;
int led2GreenPin = A11;

// Управление моторами через L298N
int motor1EnablePin = 44;  // ВКЛ/ВЫКЛ 1 мотор
int motor1PinA = 24;       // Управление 1 мотором
int motor1PinB = 26;       // Управление 1 мотором
int motor2EnablePin = 45;  // ВКЛ/ВЫКЛ 2 мотор
int motor2PinA = 28;       // Управление 2 мотором
int motor2PinB = 30;       // Управление 2 мотором

// Управление помпой
int pumpPin = 32;

// Состояния устройств
int motorA1A2State = 0; // 1 - верх, 2 - низ, 0 - стоп
int motor8_11State = 0; // 1 - верх, 2 - низ, 0 - стоп
bool servo2_3State = false;  // false - закрыто, true - открыто
bool servo4_5State = false;  // false - закрыто, true - открыто
bool led1State = false;      // false - выключен, true - включен
bool led2State = false;      // false - выключен, true - включен
int motor1Speed = 0;         // Скорость мотора 1 (0-255)
int motor2Speed = 0;         // Скорость мотора 2 (0-255)
bool pumpState = false;      // Состояние помпы

// Функция для управления сервоприводами
void controlServos(bool open2_3, bool open4_5) {
  if (open2_3) {
    servo1.write(-5); // Открыто
    servo2.write(93);
  } else {
    servo2.write(-5);
    delay(100);
    servo1.write(93); // Закрыто
  }

  if (open4_5) {
    servo3.write(-5); // Открыто
    servo4.write(93);
  } else {
    servo3.write(93); // Закрыто
    servo4.write(-5);
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

// Функция для управления моторами через L298N
void controlL298N(int speed1, int speed2) {
  // Управление направлением и скоростью мотора 1
  speed1 = map(speed1, 0, 100, 0, 255);
  if (speed1 > 0) {
    digitalWrite(motor1PinA, HIGH);
    digitalWrite(motor1PinB, LOW);
    analogWrite(motor1EnablePin, speed1);
  } else {
    digitalWrite(motor1EnablePin, LOW); // Остановка мотора 1
  }
  speed2 = map(speed2, 0, 100, 0, 255);
  // Управление направлением и скоростью мотора 2
  if (speed2 > 0) {
    digitalWrite(motor2PinA, HIGH);
    digitalWrite(motor2PinB, LOW);
    analogWrite(motor2EnablePin, speed2);
  } else {
    digitalWrite(motor2EnablePin, LOW); // Остановка мотора 2
  }
}

// Функция для управления помпой
void controlPump(bool state) {
  digitalWrite(pumpPin, state ? HIGH : LOW);
}

// Функция для обработки прерываний от датчика расхода воды
void pulseCounter() {
  pulseCount++;
}

// Функция для вывода текста на TFT-дисплей
void displayText(const String &text) {
  tft.fillScreen(ST7735_BLACK); // Очистка экрана
  tft.setCursor(0, 0);          // Установка курсора в начало
  tft.setTextColor(ST7735_WHITE); // Белый текст
  tft.setTextSize(1);           // Размер текста
  tft.println(text);            // Вывод текста
}

void setup() {
  // Инициализация TFT-дисплея
  tft.initR(INITR_BLACKTAB); // Инициализация дисплея
  tft.fillScreen(ST7735_BLACK); // Очистка экрана
  tft.setTextColor(ST7735_WHITE); // Белый текст
  tft.setTextSize(1);           // Размер текста
  tft.println("System started"); // Начальное сообщение

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

  // Инициализация моторов через L298N
  pinMode(motor1EnablePin, OUTPUT);
  pinMode(motor1PinA, OUTPUT);
  pinMode(motor1PinB, OUTPUT);
  pinMode(motor2EnablePin, OUTPUT);
  pinMode(motor2PinA, OUTPUT);
  pinMode(motor2PinB, OUTPUT);

  // Инициализация помпы
  pinMode(pumpPin, OUTPUT);

  // Инициализация датчика расхода воды
  pinMode(flowSensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, FALLING);

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

    // Вывод команды на TFT-дисплей
    displayText("Command: " + command);

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

    // Обработка команд для управления моторами
    else if (command.startsWith("MOTOR1_SPEED:")) {
      motor1Speed = command.substring(13).toInt();
      Serial.println("MOTOR1 SPEED: " + String(motor1Speed));
    } else if (command.startsWith("MOTOR2_SPEED:")) {
      motor2Speed = command.substring(13).toInt();
      Serial.println("MOTOR2 SPEED: " + String(motor2Speed));
    }

    // Управление помпой
    else if (command == "PUMP_ON") {
      pumpState = true;
      Serial.println("PUMP: ON");
    } else if (command == "PUMP_OFF") {
      pumpState = false;
      Serial.println("PUMP: OFF");
    }
  }

  // Управление устройствами
  controlServos(servo2_3State, servo4_5State);
  controlMotors(motorA1A2State, motor8_11State);
  controlLEDs(led1State, led2State);
  controlPump(pumpState);

  // Управление моторами
  controlL298N(motor1Speed, motor2Speed);

  // Обновление данных датчика каждую секунду
  static unsigned long lastMeasurement = 0;
  if (millis() - lastMeasurement >= 1000) {
    detachInterrupt(digitalPinToInterrupt(flowSensorPin));
    flowRate = pulseCount / 7.5f; // Для YF-S401: 7.5 импульсов/литр
    totalWater += flowRate / 60;
    pulseCount = 0;
    lastMeasurement = millis();
    attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, FALLING);
  }

  delay(100); // Задержка для стабильности
}