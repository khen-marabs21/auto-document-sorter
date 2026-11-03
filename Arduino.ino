#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Servo.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);


SoftwareSerial espSerial(2, 3);
Servo sorterServo;

const int greenLedPin = 6;
const int redLedPin = 7;
const int servoPin = 9;


const int SERVO_REST = 90; 
const int SERVO_ORF  = 0;    
const int SERVO_SOG  = 180; 

int orfCount = 0;
int sogCount = 0;

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);

  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  digitalWrite(greenLedPin, LOW);
  digitalWrite(redLedPin, LOW);

  sorterServo.attach(servoPin);
  sorterServo.write(SERVO_REST);

  lcd.init();
  lcd.backlight();
  updateDisplay("Ready to Sort");
}

void loop() {
  if (espSerial.available() > 0) {
    String cmd = espSerial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "ORF") {
      orfCount++;
      digitalWrite(greenLedPin, HIGH);
      digitalWrite(redLedPin, LOW);
      
      sorterServo.write(SERVO_ORF);
      updateDisplay("ORF Sorted!");

      delay(3000); 

      digitalWrite(greenLedPin, LOW);
      sorterServo.write(SERVO_REST); 
      updateDisplay("Ready to Sort");
    }
    else if (cmd == "SOG") {
      sogCount++;
      digitalWrite(greenLedPin, LOW);
      digitalWrite(redLedPin, HIGH);
      
      sorterServo.write(SERVO_SOG); 
      updateDisplay("SOG Sorted!");

      delay(3000); 

      digitalWrite(redLedPin, LOW);
      sorterServo.write(SERVO_REST);
      updateDisplay("Ready to Sort");
    }
  }
}

void updateDisplay(String statusMsg) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(statusMsg);
  lcd.setCursor(0, 1);
  lcd.print("ORF:");
  lcd.print(orfCount);
  lcd.print(" SOG:");
  lcd.print(sogCount);
}