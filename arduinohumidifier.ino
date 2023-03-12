#include <SimpleDHT.h>
#include <ArduinoBLE.h>
#include "ArduinoLowPower.h"


#define RED_PIN 2 
#define YELLOW_PIN 3 
#define GREEN_PIN 4 
#define BLUE_PIN 5 
#define DHTPIN 6 
#define WAIT_SECONDS 5

SimpleDHT11 dht(DHTPIN);
int ledPins[] = {RED_PIN, YELLOW_PIN, GREEN_PIN, BLUE_PIN};

byte currentTemp = 0;
byte currentHumidity = 0;
byte previousTemp = 0;
byte previousHumidity = 0;

BLEService environmentalService("181A");
BLEByteCharacteristic tempCharacteristic("2A6E", BLERead | BLENotify);
BLEByteCharacteristic humidityCharacteristic("2A6F", BLERead | BLENotify);
BLEBoolCharacteristic fanOn("", BLERead | BLENotify);
BLEBoolCharacteristic atomizerOn("", BLERead | BLENotify);

bool readDHT(byte* temperature, byte* humidity, byte* prevTemp, byte* prevHumidity) {
  bool updateFlag = false;
  int err = SimpleDHTErrSuccess;
  if ((err = dht.read(temperature, humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT failed, err="); Serial.print(SimpleDHTErrCode(err));
    Serial.print(","); Serial.println(SimpleDHTErrDuration(err));
    return false;
  }

  if (*prevTemp != *temperature) {
    *prevTemp = *temperature;
    updateFlag = true;
  }

  if (*prevHumidity != *humidity) {
    *prevHumidity = *humidity;
    updateFlag = true;
  }
  return updateFlag;
}

void displayBinary(byte numToShow) {
  Serial.print("Passed number "); Serial.println(numToShow);
  Serial.print("Binary "); Serial.println(String((int)numToShow, BIN));
  for (int i = 0; i < 4; i++) {
    if (bitRead(numToShow, i)==1) {
      digitalWrite(ledPins[i], LOW);
      digitalWrite(ledPins[i], HIGH);
      Serial.print("Set pin "); Serial.print(i); Serial.println(" HIGH");
    }
    else {
      digitalWrite(ledPins[i], LOW);
      Serial.print("Set pin "); Serial.print(i); Serial.println(" LOW");
    }
  }
}

void blinklights(int numTransitions, int waitBetween) {
  for (int i = 0; i < numTransitions; i++) {
    displayBinary(255);
    delay(waitBetween);
    displayBinary(0);
    delay(waitBetween);
  }
}

void ledHumidity(byte humidity) {
  if (currentHumidity < 60)
    setMask(true, false, false);
  else if ((currentHumidity >= 60) && (currentHumidity < 70 ))
    setMask(false, true, false);
  else
    setMask(false, false, true);
}

void setMask(bool red, bool yel, bool green) {
  digitalWrite(RED_PIN, LOW);
  digitalWrite(YELLOW_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  delay(500);
  digitalWrite(RED_PIN, red);
  digitalWrite(YELLOW_PIN, yel);
  digitalWrite(GREEN_PIN, green);
  
}


void setup() {
  pinMode(DHTPIN, INPUT_PULLUP);
  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  Serial.begin(9600); 
  Serial.println("Starting"); 
  Serial.println("Starting BLE");
  BLE.begin();
  BLE.setLocalName("MuhHumidor");
  BLE.setDeviceName("MuhHumidor");
  BLE.setAppearance(0x0602);
  environmentalService.addCharacteristic(tempCharacteristic);
  environmentalService.addCharacteristic(humidityCharacteristic);
  tempCharacteristic.setValue(0);
  humidityCharacteristic.setValue(0);
  BLE.addService(environmentalService);
  BLE.setAdvertisedService(environmentalService);
  BLE.advertise();
  blinklights(5, 500);
}

void loop() {
  bool needUpdate = readDHT(&currentTemp, &currentHumidity, &previousTemp, &previousHumidity);
  BLEDevice central = BLE.central();
  if (central && central.connected()) {
    digitalWrite(BLUE_PIN, HIGH);
    bool btNeedUpdate = true;
    long currentTime = 0;
    long lastReadTime = 0;
    Serial.print("Connected: "); Serial.println(central.address()); 
    while(central.connected()) {
        if (btNeedUpdate) {
          Serial.println("Updating BLE values");
          tempCharacteristic.writeValue((int)currentTemp);
          humidityCharacteristic.writeValue((int)currentHumidity);
          ledHumidity(currentHumidity);
          btNeedUpdate = false;
        }

      currentTime = millis();
      if ((lastReadTime == 0) || ((currentTime - lastReadTime) >= WAIT_SECONDS * 1000))
      {
        btNeedUpdate = readDHT(&currentTemp, &currentHumidity, &previousTemp, &previousHumidity);
        Serial.print(currentTemp); Serial.print(" *C,   ");
        Serial.print(currentHumidity); Serial.println(" RH%");
        lastReadTime = millis();
      }
    }
    Serial.println("Disconnected");
    digitalWrite(BLUE_PIN, LOW);
  }

  if (needUpdate) {
    ledHumidity(currentHumidity);
  }
  Serial.print(currentTemp); Serial.print(" *C, ");
  Serial.print(currentHumidity); Serial.println(" RH%");
  delay(WAIT_SECONDS * 1000);
  //Serial.println("Entering deep sleep");
  //LowPower.deepSleep(500);
  //LowPower.deepSleep(WAIT_SECONDS * 1000);
  //Serial.println("Exiting deep sleep");
}
