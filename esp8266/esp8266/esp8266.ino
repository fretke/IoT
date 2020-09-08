#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
 
const char* ssid = "Telia-33F95F-Greitas";
const char* password = "59994CE571";
const String email = "alfredas.kiudys@gmail.com";
const String serverAddress = "http://alfre-927d525b.localhost.run/esp8266/lightBulb/";

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

if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
 
HTTPClient http;  //Declare an object of class HTTPClient
 
http.begin("http://alfre-927d525b.localhost.run/esp8266/lightBulb/" + email);  //Specify request destination
int httpCode = http.GET();                                                                  //Send the request
 
if (httpCode > 0) { //Check the returning code
 
String payload = http.getString();   //Get the request response payload
Serial.println(payload);                     //Print the response payload

StaticJsonDocument<1000> doc;
DeserializationError err = deserializeJson(doc, payload);


if (err){
  Serial.println("parsing failed");
  Serial.println(err.c_str());
  delay(5000);
  return;
  }
 const String name = doc["ledIsOn"];
 Serial.println("The name: ");
 Serial.println(name);
 if (name == "on"){
   digitalWrite(ledPin, HIGH);
  } else {
    Serial.println("led should be off");
     digitalWrite(ledPin, LOW);
    }
 
}
 
http.end();   //Close connection
 
}
//  digitalWrite(ledPin, LOW);
delay(1000);    //Send a request every 30 seconds

}
