/*
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read data from a PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 * 
 * Example sketch/program showing how to read data from a PICC (that is: a RFID Tag or Card) using a MFRC522 based RFID
 * Reader on the Arduino SPI interface.
 * 
 * When the Arduino and the MFRC522 module are connected (see the pin layout below), load this sketch into Arduino IDE
 * then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M). When
 * you present a PICC (that is: a RFID Tag or Card) at reading distance of the MFRC522 Reader/PCD, the serial output
 * will show the ID/UID, type and any data blocks it can read. Note: you may see "Timeout in communication" messages
 * when removing the PICC from reading distance too early.
 * 
 * If your reader supports it, this sketch/program will read all the PICCs presented (that is: multiple tag reading).
 * So if you stack two or more PICCs on top of each other and present them to the reader, it will first output all
 * details of the first and then the next PICC. Note that this may take some time as all data blocks are dumped, so
 * keep the PICCs at reading distance until complete.
 * 
 * @license Released into the public domain.
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
 */

#include <SPI.h>
#include <MFRC522.h>




#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above
#include<LiquidCrystal_I2C.h>
#include<ArduinoJson.h>
#include<SoftwareSerial.h>

SoftwareSerial nodemcu(5, 6);

LiquidCrystal_I2C lcd(0x27, 16, 2);

String arrr[][5] = {{"0", "oj", "sh", "99", "hehe"}, {"1", "an", "pr", "98", "heh"}, {"2", "pa", "sah", "97", "he"}};

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
MFRC522::MIFARE_Key key;

const int Buzzer = 2;
int temp[4] = {0};
int data_card[4] = {60, 55, 163, 224};
int card1[4] = {153, 229, 131, 185};
int card2[4] = {217, 84, 137, 185};
int card3[4] = {105, 138, 136, 185};

int check_card(int arr[]){
  bool f=false;
  for (int i=0 ; i<4 ; i++){
    if (data_card[i] != arr[i]) {
      f = true;
      break;
    }
  }
  if (!f) return -1;
  f = false;
  for (int i=0 ; i<4 ; i++){
    if (card1[i] != arr[i]) {
      f = true;
      break;
    }
  }
  if (!f) return 0;
  f = false;
  for (int i=0 ; i<4 ; i++){
    if (card2[i] != arr[i]) {
      f = true;
      break;
    }
  }
  if (!f) return 1;
  return 2;
}


void setup() {
	Serial.begin(9600);		// Initialize serial communications with the PC
  nodemcu.begin(9600);
  pinMode(4, INPUT);
  pinMode(7, INPUT);
  pinMode(2, OUTPUT);
	while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	SPI.begin();			// Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522
	delay(4);				// Optional delay. Some board do need more time after init to be ready, see Readme
	mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
	Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
  lcd.init();
  // turn on the backlight
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0); //col=0 row=0
  lcd.print("Connecting to");
  lcd.setCursor(0, 1);
  lcd.print("Wifi");
  while (!digitalRead(4)) {
    continue;
    }
  if (digitalRead(7)) {
  lcd.setCursor(0,0); //col=0 row=1
  lcd.clear();
  lcd.print("Connected to ");
  lcd.setCursor(0, 1);
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
  data["hum"] = "none";
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
  prev = "";

  prev += read(0, 1, 3) + ",";
  prev += read(0, 2, 3) + ",";
  prev += read(1, 4, 7) + ",";
  prev += read(1, 5, 7) + ",";
  prev += read(1, 6, 7) + ",";
  prev += read(2, 8, 11);

  Serial.println(prev);

  mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

  digitalWrite(Buzzer, HIGH);
  delay(200);
  digitalWrite(Buzzer, LOW);
  delay(100);
  digitalWrite(Buzzer, HIGH);
  delay(200);
  digitalWrite(Buzzer, LOW);
  temp[0] = mfrc522.uid.uidByte[0];
  temp[1] = mfrc522.uid.uidByte[1];
  temp[2] = mfrc522.uid.uidByte[2];
  temp[3] = mfrc522.uid.uidByte[3];
  x = check_card(temp);
  if (x == -1) return;
  data["hum"] = prev;
  Serial.println(x);
  data.printTo(nodemcu);
  jsonBuffer.clear();
  lcd.clear();
  lcd.print("Registering ...");
  while (!digitalRead(4)) continue;
  if (digitalRead(7)){
    lcd.clear();
    lcd.print("Success");
  }
  else {
    lcd.clear();
    lcd.print("Unsuccessfull");
  }
  delay(5000);
  standby();
}


void standby(){
  Serial.println("5"); lcd.clear(); lcd.print("Wait for 5 ...");
  delay(1000);
  Serial.println("4"); lcd.clear(); lcd.print("Wait for 4 ...");
  delay(1000);
  Serial.println("3"); lcd.clear(); lcd.print("Wait for 3 ...");
  delay(1000);
  Serial.println("2"); lcd.clear(); lcd.print("Wait for 2 ...");
  delay(1000);
  Serial.println("1"); lcd.clear(); lcd.print("Wait for 1 ...");
  delay(1000);
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
    byte sector         = x;
    byte blockAddr      = y;
    byte dataBlock[]    = {"08Gate 3"};
    byte trailerBlock   = z;
    byte status;
    byte buffer[18];
    byte size = sizeof(buffer);


    // Serial.println(F("Current data in sector:"));
    // mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
    // Serial.println();

  
    // Serial.print(F("Reading data from block ")); Serial.print(blockAddr);
    // Serial.println(F(" ..."));
    // status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    // if (status != MFRC522::STATUS_OK) {
    //     Serial.print(F("MIFARE_Read() failed: "));
    //     Serial.println(mfrc522.GetStatusCodeName(status));
    // }
    // Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
    // dump_byte_array(buffer, 16); Serial.println();
    // Serial.println();

    // Serial.println(F("Authenticating again using key B..."));
    // status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
    // if (status != MFRC522::STATUS_OK) {
    //     Serial.print(F("PCD_Authenticate() failed: "));
    //     Serial.println(mfrc522.GetStatusCodeName(status));
    //     return;
    // }


    // Serial.print(F("Writing data into block ")); Serial.print(blockAddr);
    // Serial.println(F(" ..."));
    // dump_byte_array(dataBlock, 16); Serial.println();
    // status = mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
    // if (status != MFRC522::STATUS_OK) {
    //     Serial.print(F("MIFARE_Write() failed: "));
    //     Serial.println(mfrc522.GetStatusCodeName(status));
    // }
    // Serial.println();
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }

    status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    String temp = "";
    temp += (char) buffer[0];
    temp += (char) buffer[1];
    int cap = temp.toInt();
    String str = "";
    for (uint8_t i = 2; i < cap; i++)
  {
      str += (char)buffer[i];
  }
    // dump_byte_array(buffer, 16); 
    Serial.println(str);
       
    // Serial.println(F("Checking result..."));
    // byte count = 0;
    // for (byte i = 0; i < 16; i++) {
    //     if (buffer[i] == dataBlock[i])
    //         count++;
    // }
    // Serial.print(F("Number of bytes that match = ")); Serial.println(count);
    // if (count == 16) {
    //     Serial.println(F("Success :-)"));
    // } else {
    //     Serial.println(F("Failure, no match :-("));
    //     Serial.println(F("  perhaps the write didn't work properly..."));
    // }
    // Serial.println();

    return str;
      
}
