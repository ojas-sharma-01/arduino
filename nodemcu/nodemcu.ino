#include<SoftwareSerial.h>
#include<ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <HTTPSRedirect.h>
#include <SPI.h>
#include<Wire.h>
SoftwareSerial nodemcu (D6, D5);
const char* ssid     = "Wifi name";     //Set Wifi Name
const char* password = "password"; //Set Wifi Password
const char *GScriptId = "AKfycbz4sLIvn6o6vB39MknNuVi2YXAt0FsR5EIZQZ6_2rKRRzxNLivXmWKofqqcBYMecc-Edg"; //Change ID
String gate_number = "Gate Number"; //Chaneg Entry Gate According to you 
const char* host        = "script.google.com";
const int   httpsPort   = 443;
const char* fingerprint = "";
String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;
String payload_base =  "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload = "";
String student_id;

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);  pinMode(4, OUTPUT);  pinMode(5, OUTPUT);
  Serial.begin(9600);  nodemcu.begin(9600);
  WiFi.begin(ssid, password);  Serial.print("Connecting to ");  Serial.print(ssid); Serial.println(" ...");  digitalWrite(4, LOW);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("WiFi Connected!"); Serial.println(WiFi.localIP());  delay(2000);
  client = new HTTPSRedirect(httpsPort);  client->setInsecure();  client->setPrintResponseBody(true);  client->setContentTypeHeader("application/json");
  delay(5000);  Serial.print("Connecting to ");  Serial.println(host);
  bool flag = false;
  for(int i=0; i<5; i++){ 
    int retval = client->connect(host, httpsPort);
    if (retval == 1){
      flag = true; String msg = "Connected. OK"; Serial.println(msg);
      digitalWrite(5, HIGH); delay(2000); break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }
  if (!flag){
    Serial.print("Could not connect to server: "); Serial.println(host); delay(5000); digitalWrite(5, LOW);
    return;
  }

  digitalWrite(4, HIGH);
  delete client;    // delete HTTPSRedirect object
  client = nullptr; // delete HTTPSRedirect object
}

void loop() {
  digitalWrite(4, LOW); digitalWrite(BUILTIN_LED, LOW);
  DynamicJsonBuffer jsonBuffer; JsonObject& data = jsonBuffer.parseObject(nodemcu);

  if (data == JsonObject::invalid()) {
    Serial.println("Invalid Json Object"); jsonBuffer.clear();
    return;
  }

  Serial.println("Recieved:"); String da = data["hum"]; 
  Serial.println(da); upload_data(da);
}

void upload_data(String payy){
  static bool flag = false;
  if (!flag){
    client = new HTTPSRedirect(httpsPort); client->setInsecure();
    flag = true; client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }
  if (client != nullptr){
    if (!client->connected()){
      int retval = client->connect(host, httpsPort);
      if (retval != 1){
        Serial.println("Disconnected. Retrying..."); return;
      }
    }
  }
  else{Serial.println("Error creating client object!");  Serial.println("else");}
  String values = "\"" + payy; values += "\"}";
  payload = payload_base + values;

  // Publish data to Google Sheets
  Serial.println("Publishing data...");  Serial.println(payload);
  if (client->POST(url, host, payload)) {
    Serial.println("Success");  digitalWrite(5, HIGH);
  }
  else {
    Serial.println("Unsuccessfull");  digitalWrite(5, LOW);
  }
  // a delay of several seconds is required before publishing again    
  Serial.println("[TEST] delay(3000)");  digitalWrite(4, HIGH);
  delay(3000);
}
