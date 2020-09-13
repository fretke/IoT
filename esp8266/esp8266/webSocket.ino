#include <SocketIoClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Servo.h>

#define USER_SERIAL Serial

class ServoMotor {
  public:
    Servo motor;
    int servoPosition;
    int servoSpeed;
    ServoMotor (int pos, int spd, Servo someMotor, int port) {
      servoPosition = pos;
      servoSpeed = spd;
      motor = someMotor;
      motor.attach(port);
    }

    void printStatus() {
      Serial.println("Servo position: ");
      Serial.println(servoPosition);
      Serial.println("Servo speed: ");
      Serial.println(servoSpeed);

    }

    void moveToNewLocation(int newPosition) {
      if (servoPosition < newPosition) {
        for (int i = servoPosition; i <= newPosition; i++) {
          motor.write(i); // liepia servui suktis į kintamojo "pos" poziciją
          delay(servoSpeed); // laukia 15ms pasiekti pozicijai
        }
      } else {
        for (int i = servoPosition; i >= newPosition; i--) {
          motor.write(i); // liepia servui suktis į kintamojo "pos" poziciją
          delay(servoSpeed); // laukia 15ms pasiekti pozicijai
        }
      }
      servoPosition = newPosition;
    }

};

Servo firstServo;
Servo secondServo;
ServoMotor baseMotor (52, 10, firstServo, 0); // initializing first motor
ServoMotor mainMotor (0, 0, secondServo, 4);
int ledPin = 2;


const char* ssid = "Telia-33F95F-Greitas";
const char* pass = "59994CE571";
const char* uniqueBoardId = "5f574e9df11b1a4a3cc911c2";

int R = D1;
int G = D2;
int B = D3;




SocketIoClient webSocket;


void setup() {
  pinMode(ledPin, OUTPUT);


  USER_SERIAL.begin(115200);
  USER_SERIAL.print("what the fuck??");

  //  searchWiFi();
  connectWiFi();

  webSocket.begin("192.168.1.136", 8080);
  webSocket.on("connect", joinRoom);
  webSocket.on("Servo", controlled);
  webSocket.on("led", ledControl);
}

void loop() {
  webSocket.loop();

}

void joinRoom (const char* message, size_t length){
    webSocket.emit("room", "\"5f574e9df11b1a4a3cc911c2\"");
}

void ledControl(const char* message, size_t length) {
  USER_SERIAL.println(message);
  StaticJsonDocument<1000> doc;
  DeserializationError err = deserializeJson(doc, message);
  if (err) {
    Serial.println("parsing failed");
    Serial.println(err.c_str());
    delay(5000);
    return;
  }
  const String stateLED = doc["ledIsOn"];
 updateLED(stateLED);
}

void controlled(const char* message, size_t length) {
  USER_SERIAL.println(message);
  StaticJsonDocument<1000> doc;
  DeserializationError err = deserializeJson(doc, message);
  if (err) {
    Serial.println("parsing failed");
    Serial.println(err.c_str());
    delay(5000);
    return;
  }

  String servoName = doc["servoName"];
  String servoProperty = doc["property"];

  int value = doc["value"];

  if (servoName == "firstServo") {
    if (servoProperty == "pos") {
      if (value != baseMotor.servoPosition) {
        baseMotor.moveToNewLocation(value);
      }
    } else if (servoProperty == "speed") {
      baseMotor.servoSpeed = value;
    }
  } else if (servoName == "secondServo") {
    if (servoProperty == "pos") {
      if (value != mainMotor.servoPosition) {
        mainMotor.moveToNewLocation(value);
      }
    } else if (servoProperty == "speed") {
      mainMotor.servoSpeed = value;
    }
  }

}

void updateLED(String state) {
  if (state == "on") {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }
}




void searchWiFi() {
  int numberOfNetwork = WiFi.scanNetworks();
  USER_SERIAL.println("----");

  for (int i = 0; i < numberOfNetwork; i++ ) {
    USER_SERIAL.print("Network name: ");
    USER_SERIAL.println(WiFi.SSID(i));
    USER_SERIAL.print("Signal strength: ");
    USER_SERIAL.println(WiFi.RSSI(i));
    USER_SERIAL.println("--------------");
  }
}


void connectWiFi() {
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    USER_SERIAL.print(".");
    delay(1000);
  }

  USER_SERIAL.print("");
  USER_SERIAL.println("WiFi connected");
  USER_SERIAL.print("IP Address : ");
  USER_SERIAL.println(WiFi.localIP());

}
