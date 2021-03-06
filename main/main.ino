#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include "ACS712.h"
#include <EEPROM.h>

SoftwareSerial GSM_Serial(8, 7);
ACS712 acs_sensor(ACS712_30A, A0);

const int rs = 9, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


//const int sensorIn = A0;
const int voltageSensorIn = A1;
//int mVperAmp = 66; // use 100 for 20A Module and 66 for 30A Module


float AmpsRMS = 0;
float VoltRMS = 0;
float power = 0;
float maxPower = 30;
float maxVoltage = 230;
float maxCurrent = 0.3;
int warn = 0;
String num = "01755829767";
String num2 = "01743503472";
String message;

float kwh_now = 0.0f;
float maxKwh = 15.3f;
float kwh = 0.0f; //get from eeprom
uint32_t p_kwh_m_time;
uint32_t start_time;


void setup() {
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
  EEPROM.get(0, kwh);
  if (kwh == 0) {
    kwh = maxKwh;
  }
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
  calc_kwh();

  lcd.clear();
  lcd.print(VoltRMS);
  lcd.print("V");
  lcd.setCursor(9, 0);
  lcd.print(AmpsRMS);
  lcd.print("A");
  lcd.setCursor(0, 1);
  lcd.print(power);
  lcd.print("W");
  lcd.setCursor(9, 1);
  lcd.print(kwh);
  lcd.print(" U");



  if (VoltRMS > maxVoltage) {
    lcd.clear();
    lcd.print("Warning !!!");
    lcd.setCursor(0, 1);
    lcd.print(VoltRMS);
    lcd.print(" V");


    start_time = millis();
    while (VoltRMS > maxVoltage) {
      AmpsRMS = (acs_sensor.getCurrentAC() - 0.04) * 0.8;
      VoltRMS = getVoltage();
      power = AmpsRMS * VoltRMS;
      calc_kwh();
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
        message = "Warning!!! High Voltage " + (String)VoltRMS + " V. System Shuttingdown.";
        send_msg(num, message);
        delay(1000);
        message = "Customer:15X8G. High Voltage " + (String)VoltRMS + " V. System Shuttingdown.";
        send_msg(num2, message);
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
        message = "Warning!!! High Voltage " + (String)VoltRMS + " V.";
        send_msg(num, message);
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


    start_time = millis();
    while (AmpsRMS > maxCurrent) {
      AmpsRMS = (acs_sensor.getCurrentAC() - 0.04) * 0.8;
      VoltRMS = getVoltage();
      power = AmpsRMS * VoltRMS;
      calc_kwh();
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
        message = "Warning!!! High Current " + (String)AmpsRMS + " A.System shuttingdown";
        send_msg(num, message);
        digitalWrite(12, HIGH);
        delay(4000);
        message = "Customer:15X8G. High Current " + (String)AmpsRMS + " A.System shuttingdown";
        send_msg(num2, message);
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
        message = "Warning!!! High Current " + (String)AmpsRMS + " A.System may shutdown";
        send_msg(num, message);
        Serial.println("SMS Sent!");
      }
      if ((millis() - start_time) > 5000) {
        digitalWrite(11, HIGH);
        delay(10);
        digitalWrite(11, LOW);
      }
    }
  }
  else if (kwh == 0) {
    message = "Your balance is zero. Please recharge";
    send_msg(num, message);
    lcd.clear();
    lcd.print("Please Recharge");
    while (1) {
      digitalWrite(12, HIGH);
    }
  }
  Serial.print(AmpsRMS);
  Serial.print(",");
  Serial.print(VoltRMS);
  Serial.print(",");
  Serial.println(power);
}

void calc_kwh() {
  kwh_now = ((power / 1000.0) * ((millis() - p_kwh_m_time) / (1000.0 * 60 * 60))) * 30000.0;
  p_kwh_m_time = millis();
  kwh = kwh - kwh_now;
  if (kwh <= 0) {
    kwh = 0.0f;
  }
  EEPROM.put(0, kwh);

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
void send_msg(String num0, String msg)
{
  GSM_Serial.print("AT+CMGS=\"");
  GSM_Serial.print(num0);
  GSM_Serial.print("\"");
  GSM_Serial.write(13);
  GSM_Serial.write(10);//enter
  delay(1000);

  GSM_Serial.print(msg);
  GSM_Serial.write(26);//ctrl+Z
}

