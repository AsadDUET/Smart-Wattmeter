#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
//SIM800 TX is connected to Arduino D8
#define SIM800_TX_PIN 8
//SIM800 RX is connected to Arduino D7
#define SIM800_RX_PIN 7
//Create software serial object to communicate with SIM800
SoftwareSerial serialSIM800(SIM800_TX_PIN, SIM800_RX_PIN);

const int rs = 9, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


const int sensorIn = A0;
const int voltageSensorIn = A1;
int mVperAmp = 66; // use 100 for 20A Module and 66 for 30A Module


double AmpsRMS = 0;
double VoltRMS = 0;
double power = 0;
double maxPower = 28;


void setup() {
  pinMode(12, OUTPUT);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  Serial.begin(9600);
  while (!Serial);

  //Being serial communication with Arduino and SIM800
  serialSIM800.begin(9600);
  delay(1000);

  Serial.println("Setup Complete!");
  Serial.println("Sending SMS...");

  //Set SMS format to ASCII
  serialSIM800.write("AT+CMGF=1\r\n");
  delay(1000);

  //Send new SMS command and message number
  serialSIM800.write("AT+CMGS=\"01813810515\"\r\n");
  delay(1000);


  lcd.print("Initialized");
}

void loop() {
  digitalWrite(12, LOW);
  AmpsRMS = getCurrent();
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


  if (power > maxPower) {
    lcd.clear();
    lcd.print("Warning !!!");
    lcd.setCursor(0, 1);
    lcd.print(power);
    lcd.print("W");
    //Send SMS content
    serialSIM800.write("Warning !!! Too High Power consumption. System may shutdown in 1 minute");


    delay(100);
    //Send Ctrl+Z / ESC to denote SMS message is complete
    serialSIM800.write((char)26);
    delay(1000);
    Serial.println("SMS Sent!");


    uint32_t start_time = millis();
    while (power > maxPower) {
      AmpsRMS = getCurrent();
      VoltRMS = getVoltage();
      power = AmpsRMS * VoltRMS;
      lcd.clear();
      lcd.print("Warning !!!");
      lcd.setCursor(0, 1);
      lcd.print(power);
      lcd.print("W");
      lcd.setCursor(11, 1);
      lcd.print((millis() - start_time)/1000);
      lcd.print("s");
      if ((millis() - start_time) > 20000) {
        //Send SMS content
        serialSIM800.write("System is shutting down");
        delay(100);
        //Send Ctrl+Z / ESC to denote SMS message is complete
        serialSIM800.write((char)26);
        delay(1000);
        Serial.println("SMS Sent!");

        //cut line;
        while (1) {
          digitalWrite(12, HIGH);
        }
      }
    }
  }
  Serial.print(100 + AmpsRMS);
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
  /*int readValue = 0;
    float result;
    readValue = analogRead(voltageSensorIn);

    //Serial.println(readValue);
    result = ((float)readValue / (1024 - 225)) * 240;
    return result;*/
}

float getCurrent()
{
  float result;
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here

  uint32_t start_time = millis();
  while ((millis() - start_time) < 2000) //sample for 1 Sec
  {
    readValue = analogRead(sensorIn);
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
  result = ((maxValue - minValue) * 5.0) / 1024.0;
  result = (result / 2.0) * 0.707;
  result = (result * 1000) / mVperAmp;

  return result - 0.18;
}
