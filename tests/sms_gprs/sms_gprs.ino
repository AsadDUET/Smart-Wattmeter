#include <gprs.h>;
#include <SoftwareSerial.h>;
 
GPRS gprs;
 
void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("GPRS - Send SMS Test ...");
  gprs.preInit();
  delay(1000);
  while(0 != gprs.init()) {
      delay(1000);
      Serial.print("init error\r\n");
  } 
  Serial.println("Init success, start to send SMS message...");
  gprs.sendSMS("+8801813810515","hello,world"); //define phone number and text
}
 
void loop() {
  //nothing to do
}
