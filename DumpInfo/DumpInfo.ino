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
}
bool ff = false;
String prev = "";
int x;
void loop() {
  digitalWrite(7 ,HIGH);
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
  data["hum"] = String(arrr[x][0] + "," + arrr[x][1] + "," + arrr[x][2] + "," + arrr[x][3] + "," + arrr[x][4]);
  Serial.println(x);
  data.printTo(nodemcu);
  jsonBuffer.clear();
  lcd.clear();
  lcd.print("Registering ...");
  while (!digitalRead(4)) continue;
  if (digitalRead(5)){
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
