#include <SocketIoClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Servo.h>

//#define Serial Serial

class ServoMotor {
  public:
    Servo motor;
    int servoPosition;
    int servoSpeed;
    int servoPort;
    ServoMotor (int pos, int spd, Servo someMotor, int port) {
      servoPosition = pos;
      servoSpeed = spd;
      servoPort = port;
      motor = someMotor;
    }

    void initialize() {
      motor.write(servoPosition);
      motor.attach(servoPort);
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
Servo thirdServo;
Servo ESC;
ServoMotor baseMotor (0, 0, firstServo, 0); // initializing first motor
ServoMotor mainMotor (0, 0, secondServo, 4);
ServoMotor gripperMotor (0, 0, thirdServo,  5);
int ledPin = 2;


const char* ssid = "Telia-33F95F-Greitas";
const char* pass = "59994CE571";
const char* uniqueBoardId = "5f574e9df11b1a4a3cc911c2";
const char* email = "alfredas.kiudys@gmail.com";
const String endPoint = "http://192.168.1.136:8080/esp8266/lightBulb/alfredas.kiudys@gmail.com";



unsigned long currentTime;
SocketIoClient webSocket;


void setup() {
  pinMode(ledPin, OUTPUT);

  Serial.begin(115200);
  Serial.println("First Servo current Pos");
  Serial.println(baseMotor.motor.read());
  Serial.println("Second Servo current Pos");
  Serial.println(mainMotor.motor.read());

  //  searchWiFi();
  connectWiFi();
  getStartingData();
  baseMotor.printStatus();
  mainMotor.printStatus();
  webSocket.begin("192.168.1.136", 8080);
  webSocket.on("connect", joinRoom);
  webSocket.on("Servo", moveServo);
  webSocket.on("led", ledControl);
  webSocket.on("playSequence", playSequence);
}

void loop() {
  webSocket.loop();

}

void playSequence(const char* message, size_t length) {
  Serial.println("sequence to be played:");
  Serial.println(message);
  StaticJsonDocument<1000> doc;
  DeserializationError err = deserializeJson(doc, message);
  if (err) {
    Serial.println("parsing failed");
    Serial.println(err.c_str());
    delay(5000);
    return;
  }

  int numberOfMoves = doc["numberOfMoves"];
  for (int i = 0; i < numberOfMoves; i++) {
    const String servoName = doc["data"][i]["name"];
    int pos = doc["data"][i]["pos"];
    int servoSpeed = doc["data"][i]["speed"];
    Serial.println(servoName);
    if (servoName == "firstServo") {
      baseMotor.servoSpeed =  servoSpeed;
      baseMotor.moveToNewLocation(pos);
    } else if (servoName == "secondServo") {
      mainMotor.servoSpeed =  servoSpeed;
      mainMotor.moveToNewLocation(pos);
    }
    delay(200);
    //   Serial.println("Array size");
    //  Serial.println(sizeof(doc));
    //    Serial.println("element size");
    //  Serial.println(sizeof(doc[0]));
  }
  getMotorDataJson();
  //  webSocket.emit("servoPos", "\"{\"data\":\"great\"}\"" );
  int dis = 50;

  //  webSocket.emit("taskCompleted", "\"success\"");
  //  webSocket.emit("currentServoPos", getMotorDataJson());
  // DynamicJsonDocument jsonData = getMotorDataJson();
  // Serial.println("Current motor data");
  // Serial.println(jsonData);
}

String getMotorDataJson () {
  const size_t capacity = JSON_ARRAY_SIZE(3) + 3 * JSON_OBJECT_SIZE(3);
  DynamicJsonDocument doc(capacity);

  JsonObject doc_0 = doc.createNestedObject();
  doc_0["name"] = "firstServo";
  doc_0["pos"] = baseMotor.servoPosition;
  doc_0["speed"] = baseMotor.servoSpeed;

  JsonObject doc_1 = doc.createNestedObject();
  doc_1["name"] = "secondServo";
  doc_1["pos"] = mainMotor.servoPosition;
  doc_1["speed"] = mainMotor.servoSpeed;

  JsonObject doc_2 = doc.createNestedObject();
  doc_2["name"] = "thirdServo";
  doc_2["pos"] = gripperMotor.servoPosition;
  doc_2["speed"] = gripperMotor.servoSpeed;
  String data;
  serializeJson(doc, data);
  Serial.println("Current motor data");
  //   data = "\""+data+"\"";
  data = "{\"foo\":\"bar\"}";
  Serial.println(data);
  //  webSocket.emit("servoPos", ("\"" + data + "\"").c_str());
  //  webSocket.emit("servoPos", "{\"foo\":\"bar\"}");

  return data;

}

void getStartingData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(endPoint);
    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString();
      http.end();
      StaticJsonDocument<1000> doc;
      DeserializationError err = deserializeJson(doc, payload);
      if (err) {
        Serial.println("parsing failed");
        Serial.println(err.c_str());
        delay(5000);
        return;
      }
      const String stateLED = doc["ledIsOn"];
      const int newPosBase = doc["firstServo"];
      const int newPosMain = doc["secondServo"];
      Serial.print(stateLED);
      Serial.print(newPosBase);
      Serial.print(newPosMain);
      baseMotor.servoSpeed = doc["firstServoSpeed"];
      mainMotor.servoSpeed = doc["secondServoSpeed"];
      baseMotor.servoPosition = newPosBase;
      mainMotor.servoPosition = newPosMain;
      updateLED(ledPin, stateLED);

      baseMotor.initialize();
      mainMotor.initialize();
      //      if (newPosBase != baseMotor.servoPosition) {
      //        baseMotor.moveToNewLocation(newPosBase);
      //      }
      //      if (newPosMain != mainMotor.servoPosition) {
      //        mainMotor.moveToNewLocation(newPosMain);
      //      }
    }
  }
}

void updateLED(int ledPin, String state) {
  Serial.println("time on: ");
  Serial.print(millis());
  if (state == "on") {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }
}

void joinRoom (const char* message, size_t length) {
  webSocket.emit("room", "\"5f574e9df11b1a4a3cc911c2\"");
}

void ledControl(const char* message, size_t length) {
  Serial.println(message);
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
  webSocket.emit("taskCompleted", "\"success\"");
}

void moveServo(const char* message, size_t length) {
  Serial.println(message);
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

  //  switch (servoName) {
  //    case "firstServo":
  //      if (servoProperty == "pos") {
  //        if (value != baseMotor.servoPosition) {
  //          baseMotor.moveToNewLocation(value);
  //        }
  //      } else if (servoProperty == "speed") {
  //        baseMotor.servoSpeed = value;
  //      }
  //      break;
  //    case "secondServo":
  //      if (servoProperty == "pos") {
  //        if (value != mainMotor.servoPosition) {
  //          mainMotor.moveToNewLocation(value);
  //        }
  //      } else if (servoProperty == "speed") {
  //        mainMotor.servoSpeed = value;
  //      }
  //      break;
  //    case "thirdServo":
  //      if (servoProperty == "pos") {
  //        if (value != gripperMotor.servoPosition) {
  //          gripperMotor.moveToNewLocation(value);
  //        }
  //      } else if (servoProperty == "speed") {
  //        gripperMotor.servoSpeed = value;
  //      }
  //      break;
  //    default: break;
  //  }

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
  } else if (servoName == "thirdServo") {
    if (servoProperty == "pos") {
      if (value != gripperMotor.servoPosition) {
        gripperMotor.moveToNewLocation(value);
      }
    } else if (servoProperty == "speed") {
      gripperMotor.servoSpeed = value;
    }
  }

  webSocket.emit("taskCompleted", "\"success\"");

}

void updateLED(String state) {
  Serial.println("time on: ");
  currentTime = millis();
  Serial.print(currentTime);
  while (millis() < currentTime + 1000) {}
  if (state == "on") {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }
}




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
