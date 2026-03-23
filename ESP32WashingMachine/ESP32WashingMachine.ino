#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>


#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <WebsocketHandler.hpp>
#define MAX_CLIENTS 5



using namespace httpsserver;

// Include your generated certificate bytes
static const uint8_t PKEY[] = R"EOF(
-----BEGIN PRIVATE KEY-----
MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDuNcI+/Iz43zJf
4YllUwDDwnHoTWhJhl3Wr8C5e/fGLF/Y1r+r6VbxsWqTGW1fAgVsjgzEqnISXtuk
NSs6WExdpylPdr9NKmFHuqzhAAU4q/lZ09w6OwXtDhoGAobBENuES3PhUzGaJUAT
vtbD+WG6wJVogdgyyB0D+MTpZXQ1iER6+OqthKR8E2AZUZsarTWrZyZdFXt+Usfg
+qdc/wYGhCR4Hx/9dj5shhRkP6+ztE9JAbnoC3B3ccLXDxbANh881UBowsUQvixV
DMSaCx4nd1aLABdhVSqGjHRRYZJnY7auaFweAzKI5lpve54oit7PF3aUjPVFYuRq
NdeRaOU5AgMBAAECggEBAMrAyzMdd0glghUGPRIXUiFNkfKuYEOksJ6ZPasjD0SI
ZJSGROKoW66g2huWmvcxGzjIt6l1gl6Mitr7vm2M1oMoUwsmAbJGjkKla5yfjdIs
3rJMl7igoGyjghb3c4dFN/Mk3d8+FowjhKTJ3Lc1vq9cWE95pXGjzr6gscoO5pjg
XTQmp/4CdROV6fIucYAURNzBYbG2udslO32lDNbCXmwOByjDXc0RA76LCAT90eZe
BbOirR9qhbzm+/3QAV2B/CdCSKThojhsXJndQaTe/kHjPaLKcb9/fKOTZoYfvr5B
lEq7afQgfS5bCu2LyktbxTiCzGIhyzNbx97w4aQfc2ECgYEA+O52XViiYhelqq2h
TjnHamj0+s/XuJ1KZS6GsPb+Y6X8PspxMS6RfV8TkdrtjvAbzK6IB+d6Xdd3UOBK
61zGbShMc5XW3qHSGT19qVFNR1NSpy9haX1Bv0AHyBXuadnBTGu1zt4OhWIYoFVO
7RPOavjSrVH+F4jbhsygfhHO3PUCgYEA9PlcB2UCL8l/pVIroBIh9I751jae+UDq
nc/1rA2BlC9eRqDTtZY9VTvSufmYEuwAEAwKUaBp1pJyHItZascqJqkyrYdcsNR/
uc+Ux/zlXb2fELcMIabLP0cRnAsa3/ECcF8+habtphSHLKhR8G0k3Y3/FHJb/gfV
p0U6EzagfLUCgYEA7mUY8B6RLJXu7zznTQ4ifzLS4lwoAMKJ28qp7VItn+r3Xn7r
1ij95m4mRLnAZfJm/SdsWP2C/9n4h3i15x9rXGCKjutB4uZgWhG8hWm6M4OFlVzX
0nnFfE3q5eCy+aYz62b/r4STrYObh/kK5BrixSlUQe5TA+DqM/dGAFY3oP0CgYBM
upnPmxyfOR6o+l69qVBHWEz6gmsyqNmTimJpfGV8s4V028TT2HXnb3BmXb37Fz5B
yHOm83aTBZWGSUM9hZo3N8GuxnoKzNQgr9rq9NQmk1DyHFNJawO4Ext546SMReG8
rqhXllxTK5TXMPVRN+5XErW1gsg7fdq2pKE3CWIlDQKBgG1w8j3bUEtkX8SDcXYX
p+ichM5xq36KTwlEzcByxRa8lGZBUDQ+WsOfwP0UeS6codJKLcCMkU28H3/sAGJv
t8SD8//OEqYM/KwpuWt7R0VLoq+y8NRA3RNlZvhAo0WxTVdLPE0N5tXI+aHoHpQa
azohGB/ILiW1rqK6JlZZJDHs
-----END PRIVATE KEY-----
)EOF";

static const uint8_t CERTFCT[] = R"EOF(
-----BEGIN CERTIFICATE-----
MIIC0TCCAbmgAwIBAgIUWmYpnRzT9VNb7OeiOy/WgKgMPFYwDQYJKoZIhvcNAQEL
BQAwGDEWMBQGA1UEAwwNMTkyLjE2OC4xLjI1NDAeFw0yNjAzMjMwMzA1NTRaFw0z
NjAzMjAwMzA1NTRaMBgxFjAUBgNVBAMMDTE5Mi4xNjguMS4yNTQwggEiMA0GCSqG
SIb3DQEBAQUAA4IBDwAwggEKAoIBAQDuNcI+/Iz43zJf4YllUwDDwnHoTWhJhl3W
r8C5e/fGLF/Y1r+r6VbxsWqTGW1fAgVsjgzEqnISXtukNSs6WExdpylPdr9NKmFH
uqzhAAU4q/lZ09w6OwXtDhoGAobBENuES3PhUzGaJUATvtbD+WG6wJVogdgyyB0D
+MTpZXQ1iER6+OqthKR8E2AZUZsarTWrZyZdFXt+Usfg+qdc/wYGhCR4Hx/9dj5s
hhRkP6+ztE9JAbnoC3B3ccLXDxbANh881UBowsUQvixVDMSaCx4nd1aLABdhVSqG
jHRRYZJnY7auaFweAzKI5lpve54oit7PF3aUjPVFYuRqNdeRaOU5AgMBAAGjEzAR
MA8GA1UdEQQIMAaHBMCoAf4wDQYJKoZIhvcNAQELBQADggEBAMZXRYXRnWmUF68I
tZLBef4BmhqL1et+1sfdX+lGz4KZDdej5exs9CnAsP45uAzrm6ynOebpKJW4P/hQ
YWQZvWT5kuO4+0YREWuVSTTeJ9lNRxRpnx2SGOyxn/v5u6xpJB7fOsMa1YE1fEJN
PfFkpJSFXvByhtPjT3m3vz4Ck4LUKuQSurN8LK2OGAPgzqQx89MgBJy3lZgMsJkD
gf2n87+6q8cNFGsJiw+6k8Nf3fPfcKNaSgHcNlNHJT1UsZXKQ1zVYI7bs+5NG2R5
qyTFquwiNgi3x4axdJRlfgcRrPzbCz6sHaKfY1gjhENmsf4W3uwfyuKKvs9+eqsg
bCeCOOg=
-----END CERTIFICATE-----
)EOF";



// ---------------- WIFI ----------------
const char* ssid = "fsd";
const char* password = "dsa";


// ---------------- PINS ----------------
const int levelSensorPin = 14;
const int buzzerPin = 21;

const int relayMaster = 16;
const int relayInterlock = 17;
const int relayDirA = 18;
const int relayDirB = 19;


// ---------------- WEB SERVER ----------------
SSLCert cert((uint8_t*)CERTFCT, sizeof(CERTFCT), (uint8_t*)PKEY, sizeof(PKEY));
HTTPSServer secureServer(&cert);

class WSSHandler : public WebsocketHandler {
public:
  // This method is called by the webserver to instantiate a new handler for each
  // client that connects to the websocket endpoint
  static WebsocketHandler* create();

  // This method is called when a message arrives
  void onMessage(WebsocketInputStreambuf * input);

  // Handler function on connection close
  void onClose();
};



WSSHandler* activeClients[MAX_CLIENTS] = { nullptr };

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
unsigned long timeRemaining;

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
  WebsocketNode* wsNode = new WebsocketNode("/ws", WSSHandler::create);
  secureServer.registerNode(wsNode);
  secureServer.start();
  Serial.println("Secure WSS server running at wss://" + WiFi.localIP().toString());


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

secureServer.loop();

    
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
  if ((washState == IDLE || washState == COMPLETE) && tankFull && !buzzerSilenced) {
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
    timeRemaining = totalWashTime-(millis()-washStartTime);
   char buffer[50];
sprintf(buffer, "WASH_WASHING:%lu-%lu", totalWashTime, timeRemaining);
sendStatus(buffer); 
}

void stopWashCycle() {
  washState = IDLE;
  allRelaysOff();
  sendStatus("WASH_STOP");
}

void updateWashCycle() {
  if (washState == IDLE || washState == COMPLETE) return;

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


void handleCommand(String msg) {

  if (msg == "START") {
    startWashCycle();
  }

  else if (msg == "STOP") {
    stopWashCycle();
  }

  else if (msg == "SILENCE") {
    silenceBuzzer();
  }

  else if (msg == "testrelays") {
    digitalWrite(relayMaster, HIGH);
    digitalWrite(relayInterlock, HIGH);
    digitalWrite(relayDirA, HIGH);
    digitalWrite(relayDirB, HIGH);
  }

  else if (msg.startsWith("WASHTIME:")) {

    int minutes = msg.substring(9).toInt();

    if (minutes >= 1 && minutes <= 60) {

      totalWashTime = (unsigned long)minutes * 60UL * 1000UL;

      char buffer[50];
      sprintf(buffer, "WASHTIME_SET:%d", minutes);

      sendStatus(buffer);
    }
  }
};



// -----------------WEB SOCKET HANLDER ----------
WebsocketHandler * WSSHandler::create() {

    Serial.println("Client connected");

    WSSHandler* handler = new WSSHandler();

    // Store client
    for (int i = 0; i < MAX_CLIENTS; i++) {
      if (activeClients[i] == nullptr) {
        activeClients[i] = handler;
        break;
      }
    }

    // 🔥 THIS replaces your old WS_EVT_CONNECT logic
    if (!(washState == IDLE || washState == COMPLETE)) {

      unsigned long timeRemaining = totalWashTime - (millis() - washStartTime);

      char buffer[60];
      sprintf(buffer, "WASH_WASHING:%lu-%lu", totalWashTime, timeRemaining);

      handler->send(buffer, SEND_TYPE_TEXT);
    }

    return handler;
  }

  void WSSHandler::onClose() {
    Serial.println("Client disconnected");

    for (int i = 0; i < MAX_CLIENTS; i++) {
      if (activeClients[i] == this) {
        activeClients[i] = nullptr;
      }
    }
  }

void WSSHandler::onMessage(WebsocketInputStreambuf* inbuf) {

    std::ostringstream ss;
    ss << inbuf;
    std::string msg = ss.str();

    Serial.println(("WS received: " + msg).c_str());

    handleCommand(String(msg.c_str()));
  }


void sendStatus(const char* msg) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (activeClients[i] != nullptr) {
      activeClients[i]->send(msg, WebsocketHandler::SEND_TYPE_TEXT);
    }
  }
}
