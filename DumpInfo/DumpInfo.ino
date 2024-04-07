#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above
#include<LiquidCrystal_I2C.h>
#include<ArduinoJson.h>
#include<SoftwareSerial.h>
SoftwareSerial nodemcu(5, 6);
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
MFRC522::MIFARE_Key key;
const int Buzzer = 2;
int temp[4] = {0};
int data_card[4] = {60, 55, 163, 224};

int check_card(int arr[]){
  bool f=false;
  for (int i=0 ; i<4 ; i++){
    if (data_card[i] != arr[i]) {
      f = true; break;
    }
  }
  if (!f) return -1;
  return 0;
}

void setup() {
	Serial.begin(9600);	nodemcu.begin(9600);
  pinMode(4, INPUT);  pinMode(7, INPUT);  pinMode(2, OUTPUT);
	while (!Serial);	
	SPI.begin(); mfrc522.PCD_Init();		
	delay(4);				
	mfrc522.PCD_DumpVersionToSerial();	
	Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
  lcd.init();   lcd.backlight();    lcd.clear();    lcd.setCursor(0,0); 
  lcd.print("Connecting to");   lcd.setCursor(0, 1);   lcd.print("Wifi");
  while (!digitalRead(4)) {
    continue;
    }
  if (digitalRead(7)) {
  lcd.setCursor(0,0);  lcd.clear();
  lcd.print("Connected to "); lcd.setCursor(0, 1);
  lcd.print("Wifi");
  }
  for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }
    dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);
}
bool ff = false;
String prev = "";
int x;
void loop() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
	// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}
	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
    Serial.println("fdf");
		return;
	}
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  prev = "";  prev += read(0, 1, 3) + "," + read(0, 2, 3) + "," + read(1, 4, 7) + "," + read(1, 5, 7) + "," + read(1, 6, 7) + "," + read(2, 8, 11);
  Serial.println(prev);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  digitalWrite(Buzzer, HIGH); delay(200);
  digitalWrite(Buzzer, LOW); delay(100);
  digitalWrite(Buzzer, HIGH); delay(200);
  digitalWrite(Buzzer, LOW);
  temp[0] = mfrc522.uid.uidByte[0];  temp[1] = mfrc522.uid.uidByte[1];  temp[2] = mfrc522.uid.uidByte[2];  temp[3] = mfrc522.uid.uidByte[3];
  x = check_card(temp);
  if (x == -1) return;
  data["hum"] = prev;   Serial.println(x);
  data.printTo(nodemcu);   jsonBuffer.clear();
  lcd.clear();   lcd.print("Registering ...");
  while (!digitalRead(4)) continue;
  if (digitalRead(7)){
    lcd.clear(); lcd.print("Success");
  }
  else {
    lcd.clear(); lcd.print("Unsuccessfull");
  }
  delay(2000);
  standby();
}


void standby(){
  Serial.println("5"); lcd.clear(); lcd.print("Wait for 5 ..."); delay(1000);
  Serial.println("4"); lcd.clear(); lcd.print("Wait for 4 ..."); delay(1000);
  Serial.println("3"); lcd.clear(); lcd.print("Wait for 3 ..."); delay(1000);
  Serial.println("2"); lcd.clear(); lcd.print("Wait for 2 ..."); delay(1000);
  Serial.println("1"); lcd.clear(); lcd.print("Wait for 1 ..."); delay(1000);
  lcd.clear();
  lcd.print("ready");
}


void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

String read(int x, int y, int z){
    byte sector = x;   byte blockAddr = y;   byte dataBlock[] = {"08Gate 3"};  byte trailerBlock = z;
    byte status;  byte buffer[18];  byte size = sizeof(buffer);

    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: ")); Serial.println(mfrc522.GetStatusCodeName(status)); return;
    }
    status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: ")); Serial.println(mfrc522.GetStatusCodeName(status));
    }
    String temp = "";
    temp += (char) buffer[0]; temp += (char) buffer[1]; int cap = temp.toInt();
    String str = "";
    for (uint8_t i = 2; i < cap; i++) str += (char)buffer[i];
    // dump_byte_array(buffer, 16); 
    Serial.println(str);
    return str;
}
