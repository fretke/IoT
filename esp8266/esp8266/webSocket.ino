#include <SocketIoClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Servo.h>
#include <map>

class ServoMotor {
   public:
    String servoName;
    Servo motor;
    int servoPosition;
    int servoSpeed;
    int servoPort;
    ServoMotor (String servoName, int port, int servoSpeed, int servoPos) {
      this->servoName = servoName;
      this->servoPort = port;
      this->servoSpeed = servoSpeed;
      this->servoPosition = servoPos;
      this->initialize();
    }

    void initialize() {
      this->motor.write(this->servoPosition);
      this->motor.attach(this->servoPort);
    }

    void printStatus() {
      Serial.println("Servo position: ");
      Serial.println(this->servoPosition);
      Serial.println("Servo speed: ");
      Serial.println(this->servoSpeed);
    }

    void moveToNewLocation(int newPosition) {
      if (this->servoPosition < newPosition) {
        for (int i = this->servoPosition; i <= newPosition; i++) {
          this->motor.write(i);
          delay(101 - this->servoSpeed);
        }
      } else {
        for (int i = this->servoPosition; i >= newPosition; i--) {
          this->motor.write(i);
          delay(101 - this->servoSpeed);
        }
      }
      this->servoPosition = newPosition;
    }
};

typedef std::map<String, ServoMotor> MAP;

const int motor_pin_out[3] = {0, 4, 5};

MAP allMotors;
MAP::iterator it;

int ledPin = 2;

const String ssid = "Telia-33F95F-Greitas";
const String pass = "59994CE571";
const String board_id = "5f574e9df11b1a4a3cc911c2";
const String email = "alfredas.kiudys@gmail.com";

const String servoInfo = "http://192.168.1.136:8080/esp8266/servoInfo/alfredas.kiudys@gmail.com";
const char* socket_address = "192.168.1.136";

SocketIoClient webSocket;

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  connectWiFi();
  getMotorInfo();
  
  webSocket.begin(socket_address, 8080);
  webSocket.on("connect", joinRoom);
  webSocket.on("onServoUpdate", moveServo);
  webSocket.on("onDeviceToggle", ledControl);
  webSocket.on("playSequence", playSequence);
  webSocket.on("liveControl", moveServoLive);
}

void loop() {
  webSocket.loop();
}
// Parses message from websockets
StaticJsonDocument<1000> parseMessage(const char* message){
  StaticJsonDocument<1000> doc;
  DeserializationError err = deserializeJson(doc, message);
  if (err) {
    Serial.println("parsing failed");
    Serial.println(err.c_str());
    delay(5000);
  }
  return doc;
}

StaticJsonDocument<1000> parseMessage(String message){
  StaticJsonDocument<1000> doc;
  DeserializationError err = deserializeJson(doc, message);
  if (err) {
    Serial.println("parsing failed");
    Serial.println(err.c_str());
    delay(5000);
  }
  return doc;
}

// Executes a sequence of servo moves
void playSequence(const char* message, size_t length) {
  StaticJsonDocument<1000> doc = parseMessage(message);

  int numberOfMoves = doc["numberOfMoves"];
  for (int i = 0; i < numberOfMoves; i++) {
    const String servoName = doc["data"][i]["name"];
    int pos = doc["data"][i]["pos"];
    int servoSpeed = doc["data"][i]["speed"];
    
    it = allMotors.find(servoName);
    it->second.servoSpeed = servoSpeed;
    it->second.moveToNewLocation(pos);

    delay(200);
  }

  String positions;
  for (it = allMotors.begin(); it != allMotors.end(); ++it){
    Serial.println(it->second.servoName);
    positions += it->second.servoName + "," + String(it->second.servoSpeed) + "," + String(it->second.servoPosition) + "."; 
  }
  webSocket.emit("servoPos", ("{\"data\":\"" + positions + "\"}").c_str() );
}

// gets initial servo motor data and initializes required number of ServoMotors
void getMotorInfo() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(servoInfo);
    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString();
      http.end();
      
      StaticJsonDocument<1000> doc = parseMessage(payload);
      int qty = doc["servoQty"];

      for (int i = 0; i < qty; i++) {
        String servoName = doc["servos"][i]["name"];
        int pos = doc["servos"][i]["pos"];
        int servoSpeed = doc["servos"][i]["speed"];

        allMotors.insert(std::pair<String, ServoMotor>(servoName, ServoMotor(servoName, motor_pin_out[i], servoSpeed, pos)));
        Serial.println(servoName + " " + pos + " " + servoSpeed);

      }

      Serial.println("WHAT THE HELL?");
     Serial.println(allMotors.size());
    }
    http.end();
  }
}

// joins specific websocket rooms
void joinRoom (const char* message, size_t length) {
  webSocket.emit("room", ("\"" + board_id + "\"").c_str());
}

// updates LED | TODO: make it universal
void ledControl(const char* message, size_t length) {
  StaticJsonDocument<1000> doc = parseMessage(message);
  
  String stateLED = doc["ledIsOn"];
  updateLED(stateLED);
  webSocket.emit("taskCompleted", "\"success\"");
}

void updateLED(String state) {
  if (state == "on") {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }
}

// move servo with feedback
void moveServo(const char* message, size_t length) {
  StaticJsonDocument<1000> doc = parseMessage(message);

  String servoName = doc["name"];
  int speed = doc["speed"];
  int pos = doc["pos"];

  it = allMotors.find(servoName);
  it->second.servoSpeed = speed;
  it->second.moveToNewLocation(pos);

  webSocket.emit("taskCompleted", "\"success\"");
}

// servo movement without feedback to server
void moveServoLive(const char* message, size_t length) {
  StaticJsonDocument<1000> doc = parseMessage(message);

  String servoName = doc["name"];
  int speed = doc["speed"];
  int pos = doc["pos"];
  
  it = allMotors.find(servoName);
  it->second.servoSpeed = speed;
  it->second.moveToNewLocation(pos);
}

// Connecting to WIFI
void searchWiFi() {
  int numberOfNetwork = WiFi.scanNetworks();
  Serial.println("----");

  for (int i = 0; i < numberOfNetwork; i++ ) {
    Serial.print("Network name: ");
    Serial.println(WiFi.SSID(i));
    Serial.print("Signal strength: ");
    Serial.println(WiFi.RSSI(i));
    Serial.println("--------------");
  }
}

void connectWiFi() {
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("");
  Serial.println("WiFi connected");
  Serial.print("IP Address : ");
  Serial.println(WiFi.localIP());

}
