#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include "ACS712.h"

SoftwareSerial GSM_Serial(2, 3);
ACS712 acs_sensor(ACS712_30A, A0);

const int rs = 9, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


//const int sensorIn = A0;
const int voltageSensorIn = A1;
//int mVperAmp = 66; // use 100 for 20A Module and 66 for 30A Module


double AmpsRMS = 0;
double VoltRMS = 0;
double power = 0;
double maxPower = 30;
double maxVoltage = 230;
double maxCurrent = 3;
int warn = 0;
String num = "01743503472";


void setup() {
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  Serial.begin(9600);
  while (!Serial);

  //Being serial communication with Arduino and SIM800
  GSM_Serial.begin(9600);
  GSM_Serial.println("AT+CMGF=1");
  delay(1000);

  acs_sensor.calibrate();
  lcd.print("Initialized");
}

void loop() {
  warn = 0;
  digitalWrite(12, LOW);
  AmpsRMS = (acs_sensor.getCurrentAC() - 0.04) * 0.8;
  VoltRMS = getVoltage();

  power = AmpsRMS * VoltRMS;
  
  lcd.clear();
  lcd.print(VoltRMS);
  lcd.print("V");
  lcd.setCursor(9, 0);
  lcd.print(AmpsRMS);
  lcd.print("A");
  lcd.setCursor(0, 1);
  lcd.print(power);
  lcd.print("W");


  if (VoltRMS > maxVoltage) {
    lcd.clear();
    lcd.print("Warning !!!");
    lcd.setCursor(0, 1);
    lcd.print(VoltRMS);
    lcd.print(" V");


    uint32_t start_time = millis();
    while (VoltRMS > maxVoltage) {
      AmpsRMS = (acs_sensor.getCurrentAC() - 0.04) * 0.8;
      VoltRMS = getVoltage();
      power = AmpsRMS * VoltRMS;
      lcd.clear();
      lcd.print("Warning !!!");
      lcd.setCursor(0, 1);
      lcd.print(VoltRMS);
      lcd.print(" V");
      lcd.setCursor(11, 1);
      lcd.print((millis() - start_time) / 1000);
      lcd.print("s");
      if ((millis() - start_time) > 20000) {
        //Send SMS content
        send_msg(num, "System is Shutting Down");
        delay(1000);
        Serial.println("SMS Sent!");

        //cut line;
        while (1) {
          digitalWrite(12, HIGH);
        }
      }
      if (((millis() - start_time) > 5000) && warn == 0) {
        warn = 1;
        //Send SMS content
        send_msg(num, "Warning!!! High Voltage");
        Serial.println("SMS Sent!");
      }
      if ((millis() - start_time) > 5000) {
        digitalWrite(11, HIGH);
        delay(10);
        digitalWrite(11, LOW);
      }
    }
  }
  else if (AmpsRMS > maxCurrent) {
    lcd.clear();
    lcd.print("Warning !!!");
    lcd.setCursor(0, 1);
    lcd.print(AmpsRMS);
    lcd.print(" A");


    uint32_t start_time = millis();
    while (AmpsRMS > maxCurrent) {
      AmpsRMS = (acs_sensor.getCurrentAC() - 0.04) * 0.8;
      VoltRMS = getVoltage();
      power = AmpsRMS * VoltRMS;
      lcd.clear();
      lcd.print("Warning !!!");
      lcd.setCursor(0, 1);
      lcd.print(AmpsRMS);
      lcd.print(" A");
      lcd.setCursor(11, 1);
      lcd.print((millis() - start_time) / 1000);
      lcd.print("s");
      if ((millis() - start_time) > 20000) {
        //Send SMS content
        send_msg(num, "System is Shutting Down");
        delay(1000);
        Serial.println("SMS Sent!");

        //cut line;
        while (1) {
          digitalWrite(12, HIGH);
        }
      }
      if (((millis() - start_time) > 5000) && warn == 0) {
        warn = 1;
        //Send SMS content
        send_msg(num, "Warning!!! High Current");
        Serial.println("SMS Sent!");
      }
      if ((millis() - start_time) > 5000) {
        digitalWrite(11, HIGH);
        delay(10);
        digitalWrite(11, LOW);
      }
    }
  }
  else if (power > maxPower) {
    lcd.clear();
    lcd.print("Warning !!!");
    lcd.setCursor(0, 1);
    lcd.print(power);
    lcd.print(" W");


    uint32_t start_time = millis();
    while (power > maxPower) {
      AmpsRMS = (acs_sensor.getCurrentAC() - 0.04) * 0.8;
      VoltRMS = getVoltage();
      power = AmpsRMS * VoltRMS;
      lcd.clear();
      lcd.print("Warning !!!");
      lcd.setCursor(0, 1);
      lcd.print(power);
      lcd.print(" W");
      lcd.setCursor(11, 1);
      lcd.print((millis() - start_time) / 1000);
      lcd.print("s");
      if ((millis() - start_time) > 20000) {
        //Send SMS content
        send_msg(num, "System is Shutting Down");
        delay(1000);
        Serial.println("SMS Sent!");

        //cut line;
        while (1) {
          digitalWrite(12, HIGH);
        }
      }
      if (((millis() - start_time) > 5000) && warn == 0) {
        warn = 1;
        //Send SMS content
        send_msg(num, "Warning!!! High power consumption");
        Serial.println("SMS Sent!");
      }
      if ((millis() - start_time) > 5000) {
        digitalWrite(11, HIGH);
        delay(10);
        digitalWrite(11, LOW);
      }
    }
  }
  
  Serial.print(AmpsRMS);
  Serial.print(",");
  Serial.print(VoltRMS);
  Serial.print(",");
  Serial.println(power);
}

float getVoltage() {
  float result;
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here

  uint32_t start_time = millis();
  while ((millis() - start_time) < 1000) //sample for 1 Sec
  {
    readValue = analogRead(voltageSensorIn);
    // see if you have a new maxValue
    if (readValue > maxValue)
    {
      /*record the maximum sensor value*/
      maxValue = readValue;
    }
    if (readValue < minValue)
    {
      /*record the maximum sensor value*/
      minValue = readValue;
    }
  }

  // Subtract min from max
  result = ((minValue + ((maxValue - minValue) / 2.0)) / (1024)) * 220;

  return result;
  
}
void send_msg(String num, String msg)
{
  GSM_Serial.print("AT+CMGS=\"");
  GSM_Serial.print(num);
  GSM_Serial.print("\"");
  GSM_Serial.write(13);
  GSM_Serial.write(10);//enter
  delay(1000);

  GSM_Serial.print(msg);
  GSM_Serial.write(26);//ctrl+Z
}

