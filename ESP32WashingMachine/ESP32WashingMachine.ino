#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// ---------------- WIFI ----------------
const char* ssid = "a";
const char* password = "a";


// ---------------- PINS ----------------
const int levelSensorPin = 14;
const int buzzerPin = 21;

const int relayMaster = 16;
const int relayInterlock = 17;
const int relayDirA = 18;
const int relayDirB = 19;


// ---------------- WEB SERVER ----------------
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// ---------------- SENSOR / BUZZER ----------------
unsigned long lastSensorCheck = 0;
const unsigned long sensorInterval = 20;

bool tankFull = false;
bool buzzerSilenced = false;

// ---------------- WASH STATE MACHINE ----------------
enum WashState {
  IDLE,
  DIR_A,
  PAUSE_AB,
  DIR_B,
  PAUSE_BA,
  COMPLETE
};

WashState washState = IDLE;

unsigned long stateStartTime = 0;

// Default agitation parameters (ms)
unsigned long dirATime = 4000;
unsigned long dirBTime = 4000;
unsigned long pauseTime = 2000;
unsigned long totalWashTime = 5 * 60 * 1000; // 5 minutes

unsigned long washStartTime = 0;

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  Serial.println("Booting");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Pins
  pinMode(levelSensorPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);

  pinMode(relayMaster, OUTPUT);
  pinMode(relayInterlock, OUTPUT);
  pinMode(relayDirA, OUTPUT);
  pinMode(relayDirB, OUTPUT);

  // All relays off
  allRelaysOff();

  // WebSocket
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  // Simple test page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html",
      "<button onclick=\"ws.send('start')\">Start</button>"
      "<button onclick=\"ws.send('stop')\">Stop</button>"
      "<script>"
      "var ws=new WebSocket('ws://'+location.host+'/ws');"
      "</script>");
  });

  server.begin();
}

// ---------------- LOOP ----------------
void loop() {
  ArduinoOTA.handle();
  unsigned long now = millis();

  // Sensor block
  if (now - lastSensorCheck >= sensorInterval) {
    lastSensorCheck = now;
    checkLevelSensor();
  }

  // Wash cycle block
  updateWashCycle();



    
}

// ---------------- SENSOR LOGIC ----------------
void checkLevelSensor() {
  bool sensorActive = (digitalRead(levelSensorPin) == LOW);

  if (sensorActive && !tankFull) {
    tankFull = true;
    buzzerSilenced = false;
    sendStatus("TANK_FULL");
  }

  if (!sensorActive) {
    tankFull = false;
    buzzerSilenced = false;
  }

  updateBuzzer();
}

void updateBuzzer() {
  if (tankFull && !buzzerSilenced) {
    digitalWrite(buzzerPin, HIGH);
  } else {
    digitalWrite(buzzerPin, LOW);
  }
}

void silenceBuzzer() {
  buzzerSilenced = true;
  updateBuzzer();
}

// ---------------- WASH STATE MACHINE ----------------
void startWashCycle() {
  washState = DIR_A;
  stateStartTime = millis();
  washStartTime = millis();

  digitalWrite(relayMaster, HIGH);
  setDirectionA();

  sendStatus("WASH_START");
}

void stopWashCycle() {
  washState = IDLE;
  allRelaysOff();
  sendStatus("WASH_STOP");
}

void updateWashCycle() {
  if (washState == IDLE) return;

  unsigned long now = millis();

  // Check total wash duration
  if (now - washStartTime >= totalWashTime) {
    washState = COMPLETE;
    allRelaysOff();
    sendStatus("WASH_COMPLETE");
    return;
  }

  switch (washState) {

    case DIR_A:
      if (now - stateStartTime >= dirATime) {
        setAllDirectionOff();
        washState = PAUSE_AB;
        stateStartTime = now;
      }
      break;

    case PAUSE_AB:
      if (now - stateStartTime >= pauseTime) {
        setDirectionB();
        washState = DIR_B;
        stateStartTime = now;
      }
      break;

    case DIR_B:
      if (now - stateStartTime >= dirBTime) {
        setAllDirectionOff();
        washState = PAUSE_BA;
        stateStartTime = now;
      }
      break;

    case PAUSE_BA:
      if (now - stateStartTime >= pauseTime) {
        setDirectionA();
        washState = DIR_A;
        stateStartTime = now;
      }
      break;

    case COMPLETE:
      break;

    case IDLE:
    default:
      break;
  }
}

// ---------------- RELAY CONTROL ----------------
void allRelaysOff() {
  digitalWrite(relayMaster, LOW);
  digitalWrite(relayInterlock, LOW);
  digitalWrite(relayDirA, LOW);
  digitalWrite(relayDirB, LOW);
}

void setAllDirectionOff() {
  digitalWrite(relayDirA, LOW);
  digitalWrite(relayDirB, LOW);
}

void setDirectionA() {
  digitalWrite(relayMaster, HIGH);
  digitalWrite(relayInterlock, LOW); // select A
  digitalWrite(relayDirA, HIGH);
  digitalWrite(relayDirB, LOW);
}

void setDirectionB() {
  digitalWrite(relayMaster, HIGH);
  digitalWrite(relayInterlock, HIGH); // select B
  digitalWrite(relayDirA, LOW);
  digitalWrite(relayDirB, HIGH);
}

// ---------------- WEBSOCKET ----------------
void sendStatus(String msg) {
  ws.textAll(msg);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;

  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String msg = (char*)data;

    Serial.println("WS received: " + msg);

    if (msg == "start") {
      startWashCycle();
    }

        if (msg == "testrelays") {
            digitalWrite(relayMaster, HIGH);
  digitalWrite(relayInterlock, HIGH); // select A
  digitalWrite(relayDirA, HIGH);
  digitalWrite(relayDirB, HIGH);
      
    }

    if (msg == "stop") {
      stopWashCycle();
    }
  }
}

void onEvent(AsyncWebSocket * server,
             AsyncWebSocketClient * client,
             AwsEventType type,
             void * arg,
             uint8_t *data,
             size_t len) {

  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WS client #%u connected\n", client->id());
      break;

    case WS_EVT_DISCONNECT:
      Serial.printf("WS client #%u disconnected\n", client->id());
      break;

    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;

    default:
      break;
  }
}
