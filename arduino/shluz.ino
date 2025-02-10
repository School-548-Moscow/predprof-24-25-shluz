#include <Servo.h>
#include <Wire.h>

// Параметры для сервоприводов
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;

// Параметры для драйверов двигателей L9110S
int motor1Pin1 = A15;
int motor1Pin2 = A14;
int motor2Pin1 = A13;
int motor2Pin2 = A12;
int motor3Pin1 = A1;
int motor3Pin2 = A2;
int motor4Pin1 = A3;
int motor4Pin2 = A4;

// Датчики уровня воды
const uint8_t pinSensorEmpty = 41; // Пин для датчика "Пустой шлюз"
const uint8_t pinSensorFull = 42;  // Пин для датчика "Полный шлюз"
bool waterLevelEmpty = false;      // Состояние уровня воды "Пустой шлюз"
bool waterLevelFull = false;       // Состояние уровня воды "Полный шлюз"

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
//int motor1EnablePin = 44;  // ВКЛ/ВЫКЛ 1 мотор
//int motor1PinA = 24;       // Управление 1 мотором
//int motor1PinB = 26;       // Управление 1 мотором
//int motor2EnablePin = 45;  // ВКЛ/ВЫКЛ 2 мотор
//int motor2PinA = 28;       // Управление 2 мотором
//int motor2PinB = 30;       // Управление 2 мотором

int motor1RelayPin = 44;  // ВКЛ/ВЫКЛ 1 мотор
int motor2RelayPin = 45;  // ВКЛ/ВЫКЛ 2 мотор

// Управление помпой
int pumpPin = 32;

// Датчики расстояний
int PIN_TRIG_DOWN = 47;
int PIN_ECHO_DOWN = 46;
int PIN_TRIG_MIDDLE = 49;
int PIN_ECHO_MIDDLE = 48;
int PIN_TRIG_UP = 51;
int PIN_ECHO_UP = 50;

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
bool modeState = false;      // Режим работы шлюза. false - ручной, true - автоматический

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
//void controlL298N(bool motor1On, bool motor2On) {
//  // Управление мотором 1
//  if (motor1On) {
//    digitalWrite(motor1PinA, HIGH);
//    digitalWrite(motor1PinB, LOW);
//    analogWrite(motor1EnablePin, 255); // Полная скорость
//  } else {
//    digitalWrite(motor1EnablePin, LOW); // Остановка мотора 1
//  }
//
//  // Управление мотором 2
//  if (motor2On) {
//    digitalWrite(motor2PinA, HIGH);
//    digitalWrite(motor2PinB, LOW);
//    analogWrite(motor2EnablePin, 255); // Полная скорость
//  } else {
//    digitalWrite(motor2EnablePin, LOW); // Остановка мотора 2
//  }
//}

// Функция для управления помпой
void controlPump(bool state) {
  digitalWrite(pumpPin, state ? HIGH : LOW);
}

float distanse(int PIN_ECHO, int PIN_TRIG) {
  long duration, cm;

  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(5);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  duration = pulseIn(PIN_ECHO, HIGH);
  cm = (duration / 2) / 29.1;
  return cm;
}

// Функция для обработки прерываний от датчика расхода воды
void pulseCounter() {
  pulseCount++;
}

String currentMode = "manual";  // По умолчанию ручной режим

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

  // Инициализация моторов через L298N
//  pinMode(motor1EnablePin, OUTPUT);
//  pinMode(motor1PinA, OUTPUT);
//  pinMode(motor1PinB, OUTPUT);
//  pinMode(motor2EnablePin, OUTPUT);
//  pinMode(motor2PinA, OUTPUT);
//  pinMode(motor2PinB, OUTPUT);
  // Инициализация реле
  pinMode(motor1RelayPin, OUTPUT);
  pinMode(motor2RelayPin, OUTPUT);

  // Инициализация помпы
  pinMode(pumpPin, OUTPUT);

  // Инициализация датчиков расстояния
  pinMode(PIN_TRIG_DOWN, OUTPUT);
  pinMode(PIN_ECHO_DOWN, INPUT);
  pinMode(PIN_TRIG_MIDDLE, OUTPUT);
  pinMode(PIN_ECHO_MIDDLE, INPUT);
  pinMode(PIN_TRIG_UP, OUTPUT);
  pinMode(PIN_ECHO_UP, INPUT);

  // Инициализация датчиков уровня воды
  pinMode(pinSensorEmpty, INPUT_PULLUP);
  pinMode(pinSensorFull, INPUT_PULLUP);

  // Инициализация датчика расхода воды
  pinMode(flowSensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, FALLING);

  // Инициализация Serial
  Serial.begin(115200);

  digitalWrite(motor1RelayPin, HIGH);
  digitalWrite(motor2RelayPin, HIGH);
}

void full_shluz() {
  //controlServos(true, false); // открыть верхний бъеф
  //controlLEDs(true,false); // включить верхний зеленый светодиод
  //
  //while (distanse(PIN_ECHO_MIDDLE, PIN_ECHO_MIDDLE) > 10) { // держим верхний бъеф открытым пока не припарковался корабль в шлюзе
  //  delay(100);
  //}
  //controlLEDs(false,false); // переключить верхний зеленый светодиод на красный
  //controlServos(false, false); // закрыть верхний бъеф
  //  delay(2000); // задержка в 2 секунды
  //controlL298N(true,false); // откачиваем воду из шлюза
  //while (waterLevelEmpty) {
  //delay(100);
  //}
  //controlL298N(false,false); // останавливаем откачку воды из шлюза
  //
  //controlServos(false, true); // открыть нижний бъеф
  //controlLEDs(false,true); // включить нижний зеленый светодиод
  //while (distanse(PIN_ECHO_DOWN, PIN_ECHO_DOWN) > 10) { // держим нижний бъеф открытым пока не проплыл корабль из шлюза
  //  delay(100);
  //controlLEDs(false,false); // переключить нижний зеленый светодиод на красный
  //controlServos(false, false); // закрыть нижний бъеф
}

void empty_shluz() {
  //controlServos(false, true);                                 // открыть нижний бъеф
  //controlLEDs(false,true);                                    // включить нижний зеленый светодиод
  //
  //while (distanse(PIN_ECHO_MIDDLE, PIN_ECHO_MIDDLE) > 10) {   // держим верхний бъеф открытым пока не припарковался корабль в шлюзе
  //  delay(100);
  //}
  //controlLEDs(false,false);                                   // переключить верхний зеленый светодиод на красный
  //controlServos(false, false);                                // закрыть верхний бъеф
  //delay(2000);                                                // задержка в 2 секунды
  //controlL298N(false,true);                                   // наполняем водой шлюз
  //while (!waterLevelFull) {
  //  delay(100);
  //}
  //controlL298N(false,false);                                  // останавливаем наполнение водой шлюза
  //delay(2000);                                                // задержка в 2 секунды
  //controlServos(true, false);                                 // открыть верхний бъеф
  //controlLEDs(true,false);                                    // включить верхний зеленый светодиод
  //while (distanse(PIN_ECHO_UP, PIN_ECHO_UP) > 10) {           // держим нижний бъеф открытым пока не проплыл корабль из шлюза
  //  delay(100);
  //controlLEDs(false,false);                                   // переключить верхний зеленый светодиод на красный
  //controlServos(false, false);                                // закрыть верхний бъеф
}

void loop() {
  // Обработка сигналов с датчиков уровня воды
  waterLevelEmpty = !digitalRead(pinSensorEmpty); // Если уровень воды низкий, waterLevelEmpty = true
  waterLevelFull = !digitalRead(pinSensorFull);   // Если уровень воды высокий, waterLevelFull = true

  // Обработка команд от Serial
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    // Выбор режима работы
    if (command == "MODE_STATE_FALSE") {
      modeState = false;
      Serial.println("MODE: HAND");
    } else if (command == "MODE_STATE_TRUE") {
      modeState = true;
      Serial.println("MODE: AUTO");
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
//    else if (command == "MOTOR1_ON") {
//      motor1Speed = 255; // Включить мотор 1
//      Serial.println("MOTOR1: ON");
//    } else if (command == "MOTOR1_OFF") {
//      motor1Speed = 0; // Выключить мотор 1
//      Serial.println("MOTOR1: OFF");
//    } else if (command == "MOTOR2_ON") {
//      motor2Speed = 255; // Включить мотор 2
//      Serial.println("MOTOR2: ON");
//    } else if (command == "MOTOR2_OFF") {
//      motor2Speed = 0; // Выключить мотор 2
//      Serial.println("MOTOR2: OFF");
//    }

      else if (command == "MOTOR1_ON") {
      digitalWrite(motor1RelayPin, LOW); // Включить мотор 1
      Serial.println("MOTOR1: ON");
    } else if (command == "MOTOR1_OFF") {
      digitalWrite(motor1RelayPin, HIGH); // Выключить мотор 1
      Serial.println("MOTOR1: OFF");
    } else if (command == "MOTOR2_ON") {
      digitalWrite(motor2RelayPin, LOW);; // Включить мотор 2
      Serial.println("MOTOR2: ON");
    } else if (command == "MOTOR2_OFF") {
      digitalWrite(motor2RelayPin, HIGH); // Выключить мотор 2
      Serial.println("MOTOR2: OFF");
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
  //controlL298N(motor1Speed > 0, motor2Speed > 0);

  // Обработка сигналов с датчиков уровня воды
  waterLevelEmpty = !digitalRead(pinSensorEmpty); // Если уровень воды низкий, waterLevelEmpty = true
  waterLevelFull = !digitalRead(pinSensorFull);   // Если уровень воды высокий, waterLevelFull = true

  // Обновление данных датчиков каждую секунду
  static unsigned long lastMeasurement = 0;
  if (millis() - lastMeasurement >= 1000) {
    detachInterrupt(digitalPinToInterrupt(flowSensorPin));
    flowRate = pulseCount / 7.5f; // Для YF-S401: 7.5 импульсов/литр
    totalWater += flowRate / 60;
    pulseCount = 0;
    lastMeasurement = millis();
    attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, FALLING);
  }
  // отправка данных одной строкой
  String data_to_serial = String(distanse(PIN_ECHO_DOWN, PIN_TRIG_DOWN)) + ",";
  data_to_serial += String(distanse(PIN_ECHO_MIDDLE, PIN_TRIG_MIDDLE)) + ",";
  data_to_serial += String(distanse(PIN_ECHO_UP, PIN_TRIG_UP)) + ",";
  data_to_serial += String(totalWater) + ",";
  data_to_serial += String(flowRate) + ",";
  data_to_serial += String(!waterLevelEmpty ? "high" : "low") + ",";
  data_to_serial += String(waterLevelFull ? "high" : "low");

  delay(100);
  Serial.println(data_to_serial);

//  if (modeState){
//    if (distanse(PIN_ECHO_UP, PIN_TRIG_UP) < 10.0) {
//    full_shluz();
//    }
//  }

  delay(500); // Задержка для стабильности
}