#include<SoftwareSerial.h>
#include<ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <HTTPSRedirect.h>
#include <SPI.h>
#include<Wire.h>

SoftwareSerial nodemcu (D6, D5);
const char* ssid     = "Galaxy M040501";     //Set Wifi Name
const char* password = "ojas@2004"; //Set Wifi Password
const char *GScriptId = "AKfycbz4sLIvn6o6vB39MknNuVi2YXAt0FsR5EIZQZ6_2rKRRzxNLivXmWKofqqcBYMecc-Edg"; //Change ID
String gate_number = "Gate Number"; //Chaneg Entry Gate According to you 
const char* host        = "script.google.com";
const int   httpsPort   = 443;
const char* fingerprint = "";
String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;
String payload_base =  "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload = "";

// Declare variables that will be published to Google Sheets
String student_id;

#define stats_led D2;


void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  Serial.begin(9600);
  nodemcu.begin(9600);
  WiFi.begin(ssid, password);             
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");
    digitalWrite(4, LOW);


  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println('\n');
  Serial.println("WiFi Connected!");

  
  //Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  delay(2000);

  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  //----------------------------------------------------------
  delay(5000);
  //----------------------------------------------------------
  Serial.print("Connecting to ");
  Serial.println(host);
  //----------------------------------------------------------
  // Try to connect for a maximum of 5 times
  bool flag = false;
  for(int i=0; i<5; i++){ 
    int retval = client->connect(host, httpsPort);
    if (retval == 1){
      flag = true;
      String msg = "Connected. OK";
      Serial.println(msg);
      digitalWrite(5, HIGH);
      delay(2000);
      break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }
  //----------------------------------------------------------
  if (!flag){
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    delay(5000);
    digitalWrite(5, LOW);
    return;
  }

  digitalWrite(4, HIGH);
  //----------------------------------------------------------
  delete client;    // delete HTTPSRedirect object
  client = nullptr; // delete HTTPSRedirect object
  //----------------------------------------------------------
}

void loop() {
  digitalWrite(4, LOW);
  // put your main code here, to run repeatedly:
  digitalWrite(BUILTIN_LED, LOW);
   DynamicJsonBuffer jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject(nodemcu);

  if (data == JsonObject::invalid()) {
    Serial.println("Invalid Json Object");
    jsonBuffer.clear();
    return;
  }

  Serial.println("Recieved:");
  String da = data["hum"];
  Serial.println(da);
  upload_data(da);
}

void upload_data(String payy){
  static bool flag = false;
  if (!flag){
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }
  if (client != nullptr){
    //when below if condition is TRUE then it takes more time then usual, It means the device 
    //is disconnected from the google sheet server and it takes time to connect again
    if (!client->connected()){
      int retval = client->connect(host, httpsPort);
      if (retval != 1){
        Serial.println("Disconnected. Retrying...");
        return; //Reset the loop
      }
    }
  }
  else{Serial.println("Error creating client object!"); Serial.println("else");}
  //----------------------------------------------------------------
  //Serial.println("[TEST] Scan Your Tag");
  /* Initialize MFRC522 Module */
  /* Look for new cards */
  /* Reset the loop if no new card is present on RC522 Reader */
  /* Read data from the same block */  
  //----------------------------------------------------------------
  String values = "\"" + payy + ",";
  values += gate_number + "\"}";
  payload = payload_base + values;
  //----------------------------------------------------------------
  //----------------------------------------------------------------
  // Publish data to Google Sheets
  Serial.println("Publishing data...");
  Serial.println(payload);
  if (client->POST(url, host, payload)) {
    Serial.println("Success");
    digitalWrite(5, HIGH);
  }
  else {
    Serial.println("Unsuccessfull");
    digitalWrite(5, LOW);
  }
  //----------------------------------------------------------------
  // a delay of several seconds is required before publishing again    
  Serial.println("[TEST] delay(3000)");
  digitalWrite(4, HIGH);
  delay(3000);
}
