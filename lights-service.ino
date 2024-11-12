#include <ArduinoBLE.h> // BLE library

const int ledPin = 13; // Change this to your LED pin

int redPin = 9;
int yellowPin = 10;
int greenPin = 11;  
int bluePin = 12;


int xSensorPin = A0;   // select the input pin for the potentiometer
int ySensorPin = A1;

int swPin = 2;

int xLastSensorValue = 512;
int yLastSensorValue = 512;
int lastSWValue = -1;

BLEService lightService("cae55316-6575-413c-9858-0f76c95f6590"); // Replace with your service UUID
BLEByteCharacteristic lightCharacteristic("bd13c171-c34c-46ef-9078-70fd20719d31", BLEWrite | BLERead);

BLECharacteristic joystickCharacteristic("629e077d-fa31-47b1-a681-a0fe99dd8493", BLERead | BLENotify, 5);

unsigned long previousMillis = 0;
const long interval = 30; // Update every 100 ms

void setup() {
  Serial.begin(9600);

  pinMode(ledPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  
  pinMode(swPin, INPUT_PULLUP);

  BLE.begin();
  BLE.setLocalName("Arduino Light Controller");
  BLE.setAdvertisedService(lightService);
  lightService.addCharacteristic(lightCharacteristic);
  lightService.addCharacteristic(joystickCharacteristic);

  BLE.addService(lightService);
  lightCharacteristic.writeValue(0); // Start with LED off
  BLE.setAdvertisingInterval(100);
  BLE.advertise();

  digitalWrite(ledPin, LOW);
}

void loop() {
  Serial.print("trying to connect ...");
  Serial.print("\n");
  BLEDevice central = BLE.central();
  
  if (central) {
    digitalWrite(ledPin, HIGH);
    while (central.connected()) {
      if (lightCharacteristic.written()) {
        int value = lightCharacteristic.value();
        Serial.print("value: ");
        Serial.print(value);
        Serial.print(", & 2 = ");
        Serial.print(value & 2);
        Serial.print(", & 1 = ");
        Serial.print(value & 1);
        Serial.print("\n");
        digitalWrite(redPin, value & 1 ? HIGH : LOW);     // Bit 0 controls red light
        digitalWrite(yellowPin, value & 2 ? HIGH : LOW);  // Bit 1 controls yellow light
        digitalWrite(bluePin, value & 4 ? HIGH : LOW);    // Bit 2 controls blue light
        digitalWrite(greenPin, value & 8 ? HIGH : LOW); 
      }
      
      

      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        int xSensorValue = analogRead(xSensorPin);
        int ySensorValue = analogRead(ySensorPin);

        int swPinValue = digitalRead(swPin);
        

        if(xSensorValue != xLastSensorValue || ySensorValue != yLastSensorValue || lastSWValue != swPinValue) {
          xLastSensorValue = xSensorValue;
          yLastSensorValue = ySensorValue;
          byte joystickData[5];
          joystickData[0] = lowByte(xSensorValue);  // x low byte
          joystickData[1] = highByte(xSensorValue); // x high byte
          joystickData[2] = lowByte(ySensorValue);  // y low byte
          joystickData[3] = highByte(ySensorValue); // y high byte
          joystickData[4] = swPinValue == HIGH ? 0 : 1; 

          joystickCharacteristic.writeValue(joystickData, 5);
          Serial.print("x: ");
          Serial.print(xSensorValue);
          Serial.print(" y: ");
          Serial.print(ySensorValue);
          Serial.print(" button: ");
          Serial.println(swPinValue == HIGH ? "released" : "pressed");
        }
      }
    }
  } else {
    digitalWrite(ledPin, LOW);
  }
}
