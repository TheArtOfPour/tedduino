//Tedduino101
//@todo : description and lic
//@todo : tedometer?
//@todo : rm motor
//@todo : add config file for uuid and pins ... and songs

#include <Wire.h>
#include <CurieBLE.h>
#include "CurieIMU.h"
bleServiceName = "Tedduino"; // You're device will appear in bluetooth connection lists under this name
serviceUUID = "input your UUID here";
characteristicUUID = "input your UUID here";
BLEPeripheral blePeripheral;
BLEService bleService(serviceUUID);
BLEUnsignedCharCharacteristic bleCharacteristic(characteristicUUID, BLERead | BLEWrite);

#include "rgb_lcd.h"
rgb_lcd lcd;
const int B = 3975;
const int pinTouch = 2;
const int pinBuzzer = 3;
const int pinButton = 4;
const int pinVibe = 5;
const int pinLED = 13;
const int pinSound = A0;
const int pinLight = A1;
const int pinTemp = A2;
const int pinPot = A3;

#define IN1  8
#define IN2  9
#define IN3  10
#define IN4  11
int Steps = 0;
boolean Direction = true;// gre
unsigned long last_time;
unsigned long currentMillis;
int steps_left=4095;
long time;

int menu = 0;
int previousButtonState = 0;
int length = 15; // the number of notes
char notes[] = "ccggaagffeeddc "; // a space represents a rest
int beats[] = { 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 4 };
int tempo = 300;
float aix, aiy, aiz;   // accelerometer values
float gix, giy, giz;   // gyroscope values

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  pinMode(pinLED, OUTPUT);
  pinMode(pinPot, INPUT);
  pinMode(pinTouch, INPUT);
  pinMode(pinButton, INPUT);
  pinMode(IN1, OUTPUT); 
  pinMode(IN2, OUTPUT); 
  pinMode(IN3, OUTPUT); 
  pinMode(IN4, OUTPUT); 
  blePeripheral.setLocalName(bleServiceName);
  blePeripheral.setAdvertisedServiceUuid(bleService.uuid());
  blePeripheral.addAttribute(bleService);
  blePeripheral.addAttribute(bleCharacteristic);
  bleCharacteristic.setValue(0);
  blePeripheral.begin();
  
  CurieIMU.begin();
  CurieIMU.setGyroRate(25);
  CurieIMU.setAccelerometerRate(25);
  CurieIMU.setAccelerometerRange(2);
  CurieIMU.setGyroRange(250);
}

void loop() {
  printGyroAccel();
  digitalWrite(pinLED, LOW);
  BLECentral central = blePeripheral.central();
  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address()); //central's MAC address
    while (central.connected()) {
      digitalWrite(pinLED, HIGH);
      resetLCD();
      if (bleCharacteristic.written()) {
        Serial.println(bleCharacteristic.value());
        if (bleCharacteristic.value() == 1) {
          Serial.println("Toggle");
          nextMenu();
        } 
        else if (bleCharacteristic.value() == 3) {
          Serial.println("Motor");
          motor();
        }
        else {
          Serial.println("Play Sound");
          playSong();
        }
      }      
      displayMenu();
    }
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
  //displayMenu();
  delay(100);
}

void nextMenu() {  
  if (menu == 5) {
    menu = 0;
  }
  else {
    menu++;
  }
}

void displayMenu() {
  resetLCD();
  
  int buttonState = digitalRead(pinButton);
  int touchState = digitalRead(pinTouch);
  buttonState = buttonState | touchState;
  if (previousButtonState != buttonState) {    
    if (buttonState == 1) {
      nextMenu();
    }
  }
  previousButtonState = buttonState;

  if (menu == 0) {
    printSound();
  }
  else if (menu == 1) {
    printLight();
  }
  else if (menu == 2) {
    printTemp();
  }
  else if (menu == 3) {
    printVibe();
  }
  else if (menu == 4) {
    printPot();
  }
  else if (menu == 5) {
    nightMode();
  }
  else {
    lcd.print("Error");
  }
}

void resetLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
}

void printGyroAccel() {
  CurieIMU.readAccelerometerScaled(aix, aiy, aiz);
  CurieIMU.readGyroScaled(gix, giy, giz);
  //CurieIMU.readMotionSensor(aix, aiy, aiz, gix, giy, giz);
  lcd.print(aix);
  lcd.print(' ');
  lcd.print(aiy);
  lcd.print(' ');
  lcd.print(aiz);
  lcd.setCursor(0, 1);
  lcd.print(gix);
  lcd.print(' ');
  lcd.print(giy);
  lcd.print(' ');
  lcd.print(giz);
  delay(100);
  resetLCD();
}

int getSound() {
  int sensorValue = analogRead(pinSound);
  return sensorValue;
}

void printSound() {
  int sensorValue = getSound();
  lcd.print("Sound:");
  if (sensorValue > 0) {
    for (int i = 0; i < round(sensorValue/100); i++) {
      lcd.print(">");
    }
  }  
}

int getLight() {
  int sensorValue = analogRead(pinLight);
  return sensorValue;
}

void printLight() {
  // range 0 - 1023
  int sensorValue = getLight();
  lcd.print("Light:");
  if (sensorValue > 0) {
    for (int i = 0; i < round(sensorValue/100); i++) {
      lcd.print(">");
    }
  }  
}

int getTemp() {
  int sensorValue = analogRead(pinTemp);
  float resistance = (float)(1023-sensorValue)*10000/sensorValue;
  int temperature = 1/(log(resistance/10000)/B+1/298.15)-273.15;
  return temperature;
}

void printTemp() {
  lcd.print("Temp:");
  lcd.print(getTemp());
  lcd.print("F");
}

int getPot() {  
  return analogRead(pinPot);
}

void printPot() {
  lcd.print("Pot:");
  lcd.print(getPot());
}

int getVibe() {  
  return digitalRead(pinVibe);
}

void printVibe() {
  lcd.print("Vibe:");
  lcd.print(getVibe());
}

void nightMode() {
  bool warm = false;
  bool dark = false;
  bool quiet = false;
  
  if (getTemp() >= 50) {
    lcd.print("Warm ");
    warm = true;
  }
  else {
    lcd.print("Cold ");
  }
  if (getLight() == 0) {
    lcd.print("Dark ");
    dark = true;
  }
  else {
    lcd.print("Light ");
  }
  if (getSound() < 100) {
    lcd.print("Quiet ");
    quiet = true;
  }
  else {
    lcd.print("Loud ");
  }

  if (warm && dark && quiet) {
    playSong();
  }
}

void playSong() {  
  for (int i = 0; i < length; i++) {
    if (notes[i] == ' ') {
      delay(beats[i] * tempo); // rest
    }
    else {
      playNote(notes[i], beats[i] * tempo);
    }

    // pause between notes
    delay(tempo / 2);
  }
}

void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(pinBuzzer, HIGH);
    delayMicroseconds(tone);
    digitalWrite(pinBuzzer, LOW);
    delayMicroseconds(tone);
  }
}

void playNote(char note, int duration) {
  char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
  int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };

  // play the tone corresponding to the note name
  for (int i = 0; i < 8; i++) {
    if (names[i] == note) {
      playTone(tones[i], duration);
    }
  }
}



void motor() {  
  while(steps_left>0){
    currentMillis = micros();
    if(currentMillis-last_time>=1000){
      stepper(1); 
      time=time+micros()-last_time;
      last_time=micros();
      steps_left--;
    }
  }
  Direction=!Direction;
  steps_left=4095;
}

void stepper(int xw){
  for (int x=0;x<xw;x++){
    switch(Steps){
       case 0:
         digitalWrite(IN1, LOW); 
         digitalWrite(IN2, LOW);
         digitalWrite(IN3, LOW);
         digitalWrite(IN4, HIGH);
       break; 
       case 1:
         digitalWrite(IN1, LOW); 
         digitalWrite(IN2, LOW);
         digitalWrite(IN3, HIGH);
         digitalWrite(IN4, HIGH);
       break; 
       case 2:
         digitalWrite(IN1, LOW); 
         digitalWrite(IN2, LOW);
         digitalWrite(IN3, HIGH);
         digitalWrite(IN4, LOW);
       break; 
       case 3:
         digitalWrite(IN1, LOW); 
         digitalWrite(IN2, HIGH);
         digitalWrite(IN3, HIGH);
         digitalWrite(IN4, LOW);
       break; 
       case 4:
         digitalWrite(IN1, LOW); 
         digitalWrite(IN2, HIGH);
         digitalWrite(IN3, LOW);
         digitalWrite(IN4, LOW);
       break; 
       case 5:
         digitalWrite(IN1, HIGH); 
         digitalWrite(IN2, HIGH);
         digitalWrite(IN3, LOW);
         digitalWrite(IN4, LOW);
       break; 
         case 6:
         digitalWrite(IN1, HIGH); 
         digitalWrite(IN2, LOW);
         digitalWrite(IN3, LOW);
         digitalWrite(IN4, LOW);
       break; 
       case 7:
         digitalWrite(IN1, HIGH); 
         digitalWrite(IN2, LOW);
         digitalWrite(IN3, LOW);
         digitalWrite(IN4, HIGH);
       break; 
       default:
         digitalWrite(IN1, LOW); 
         digitalWrite(IN2, LOW);
         digitalWrite(IN3, LOW);
         digitalWrite(IN4, LOW);
       break; 
    }
    SetDirection();
  }
}

void SetDirection(){
  if(Direction==1){ Steps++;}
  if(Direction==0){ Steps--; }
  if(Steps>7){Steps=0;}
  if(Steps<0){Steps=7; }
}

