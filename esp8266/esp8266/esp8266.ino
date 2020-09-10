#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Servo.h>

const char* ssid = "Telia-33F95F-Greitas";
const char* password = "59994CE571";
const String email = "alfredas.kiudys@gmail.com";
const String serverAddress = "http://alfre-927d525b.localhost.run/esp8266/lightBulb/";

// Servo motor class for better management of servo motor movement
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

void setup () {

  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Connecting..");
  }
}

void loop() {
  
  baseMotor.printStatus();

  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
    String payload = getDataFromServer(); //Getting response from server as a string type
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
    baseMotor.servoSpeed = doc["firstServoSpeed"];
    mainMotor.servoSpeed = doc["secondServoSpeed"];
    updateLED(ledPin, stateLED);

    if (newPosBase != baseMotor.servoPosition) {
      baseMotor.moveToNewLocation(newPosBase);
    }
     if (newPosMain != mainMotor.servoPosition) {
      mainMotor.moveToNewLocation(newPosMain);
    }
  }
  delay(1000);
}

void updateLED(int ledPin, String state) {
  if (state == "on") {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }
}

String getDataFromServer () {
  HTTPClient http;  //Declare an object of class HTTPClient

  http.begin("http://alfre-98fe9057.localhost.run/esp8266/lightBulb/" + email);  //Specify request destination
  int httpCode = http.GET();                                                                  //Send the request

  if (httpCode > 0) { //Check the returning code

    String payload = http.getString();   //Get the request response payload
    http.end();   //Close connection

    return payload;
  }
  http.end();
  return "error";
}
