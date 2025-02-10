#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "shluz_boat"; // Имя точки доступа
const char* password = "12344321"; // Пароль точки доступа

ESP8266WebServer server(80);

const int motorPin1 = D3; // Пин D3 для управления мотором
const int motorPin2 = D4; // Пин D4 для управления мотором

int motorSpeed = 0; // Текущая скорость мотора (0-255)
String motorDirection = "Стоп"; // Текущее направление мотора

void setup() {
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);

  // Запуск точки доступа
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");
  Serial.println("IP Address: " + WiFi.softAPIP().toString());

  // Настройка сервера
  server.on("/", handleRoot);
  server.on("/forward", handleForward);
  server.on("/backward", handleBackward);
  server.on("/stop", handleStop);
  server.on("/setSpeed", handleSetSpeed);
  server.on("/getStatus", handleGetStatus); // Добавляем обработчик для получения статуса

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  updateMotor(); // Обновляем состояние мотора
}

void handleRoot() {
  String html = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Управление мотором</title>
  <style>
    body {
      display: flex;
      flex-direction: column;
      align-items: center;
      margin: 0;
      font-family: Arial, sans-serif;
      padding-top: 20px; /* Отступ сверху */
    }
    .controls {
      display: flex;
      flex-direction: column;
      gap: 10px;
      margin-bottom: 20px;
    }
    .controls button {
      width: 150px;
      padding: 10px;
      font-size: 16px;
    }
    .slider {
      width: 80%;
      margin-bottom: 20px;
    }
    .status {
      font-size: 18px;
      margin-bottom: 20px;
    }
  </style>
  <script>
    function sendCommand(url) {
      fetch(url)
        .then(response => response.text())
        .then(data => {
          console.log(data);
          updateStatus();
        })
        .catch(error => console.error('Error:', error));
    }

    function setSpeed() {
      const speed = document.getElementById('speedSlider').value;
      fetch('/setSpeed?speed=' + speed)
        .then(response => response.text())
        .then(data => console.log(data))
        .catch(error => console.error('Error:', error));
    }

    function updateStatus() {
      fetch('/getStatus')
        .then(response => response.text())
        .then(data => {
          document.getElementById('status').textContent = 'Режим: ' + data;
        })
        .catch(error => console.error('Error:', error));
    }

    // Обновляем статус при загрузке страницы
    window.onload = updateStatus;
  </script>
</head>
<body>
  <h1>Управление мотором</h1>
  <div class="status" id="status">Режим: Стоп</div>
  <div class="controls">
    <button onclick="sendCommand('/forward')">Вперед</button>
    <button onclick="sendCommand('/stop')">Стоп</button>
    <button onclick="sendCommand('/backward')">Назад</button>
  </div>
  <div>
    <label for="speedSlider">Скорость:</label>
    <input type="range" id="speedSlider" min="0" max="255" value="0" oninput="setSpeed()">
    <span id="speedValue">0</span>%
  </div>
  <script>
    const speedSlider = document.getElementById('speedSlider');
    const speedValue = document.getElementById('speedValue');
    speedSlider.addEventListener('input', () => {
      speedValue.textContent = Math.round((speedSlider.value / 255) * 100);
    });
  </script>
</body>
</html>
)=====";
  server.send(200, "text/html", html);
}

void handleForward() {
  motorDirection = "Вперед";
  server.send(200, "text/plain", motorDirection);
}

void handleBackward() {
  motorDirection = "Назад";
  server.send(200, "text/plain", motorDirection);
}

void handleStop() {
  motorDirection = "Стоп";
  server.send(200, "text/plain", motorDirection);
}

void handleSetSpeed() {
  if (server.hasArg("speed")) {
    motorSpeed = server.arg("speed").toInt();
    server.send(200, "text/plain", "Speed set to " + String(motorSpeed));
  } else {
    server.send(400, "text/plain", "Speed parameter missing");
  }
}

void handleGetStatus() {
  server.send(200, "text/plain", motorDirection);
}

void updateMotor() {
  if (motorDirection == "Вперед") {
    analogWrite(motorPin1, motorSpeed);
    analogWrite(motorPin2, 0);
  } else if (motorDirection == "Назад") {
    analogWrite(motorPin1, 0);
    analogWrite(motorPin2, motorSpeed);
  } else {
    analogWrite(motorPin1, 0);
    analogWrite(motorPin2, 0);
  }
}