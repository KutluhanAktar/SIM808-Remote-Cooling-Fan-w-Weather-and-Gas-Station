         ///////////////////////////////////////////////////////  
        //             SIM808 Remote Cooling Fan             //
       //             w/ Weather and Gas Station            //
      //           -------------------------------         //
     //                     Arduino Uno                   //           
    //                   by Kutluhan Aktar               // 
   //                                                   //
  ///////////////////////////////////////////////////////

// Via SMS messages; control a fan and observe the temperature, humidity, and gas status of your room.
//
// For more information about this project:
// https://www.theamplituhedron.com/projects/SIM808-Remote-Cooling-Fan-with-Weather-and-Gas-Station/
//
// Connections
// Arduino Uno:           
//                               SIM808 GPS/GPRS/GSM Shield For Arduino
// D0 --------------------------- RX
// D1 --------------------------- TX
// D12 -------------------------- POWER
//                               3-wire Serial LCD Module
// D2 --------------------------- SCK
// D7 --------------------------- CS
// D10 -------------------------- SID
// 5V --------------------------- VCC
// GND -------------------------- GND
//                               DHT1l Temperature and Humidity Sensor
// D3 --------------------------- S
//                               MQ-135 Gas Sensor
// A0 --------------------------- A0
//                               2-Way Relay Module
// D4 --------------------------- IN1


// Include required libraries:
#include <DFRobot_sim808.h>
#include "LCD12864RSPI.h"
#include "DHT.h"

// Define the sim808.
DFRobot_SIM808 sim808(&Serial);

// Define settings to read SMS messages.
#define MESSAGE_LENGTH 20
char gprsBuffer[64];
char *s = NULL;

// Define a phone number to be able to receive feedback messages from the device.
#define PHONE_NUMBER "[Enter Phone Number]"  // Change

// Define the DHT11 object.
DHT dht;

// Define pin numbers for modules.
#define mq_135 A0
#define relay 4

// Define the data holders.
const int message_size = 35;
char Temperature[message_size], Humidity[message_size], Gas[message_size];

void setup() {
  Serial.begin(9600);

  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);

  // Initiate the DHT11 Temperature and Humidity Sensor.
  dht.setup(3);

  // Initiate the SPI LCD Screen with the given pins (2, 7, 10).
  LCDA.initDriverPin(2,7,10); 
  LCDA.Initialise(); // INIT SCREEN  
  delay(100);

  //******** Initialize sim808 module *************
  while(!sim808.init()) {
     delay(1000);
     Serial.print("Sim808 init error\r\n");
     LCDA.DisplayString(0,0,"Init Error",12);
  }
  delay(2000);
  LCDA.CLEAR();
  // Continue if the SIM808 Module is working accurately.
  Serial.println("Sim808 init success");
  LCDA.DisplayString(0,0,"Init Success",12);
  delay(5000);
  LCDA.CLEAR();
  
}

void loop() {
  
  read_SMS_Messages();

}

void get_Sensor_Variables(){
  // Get data from the DHT11 Sensor.
  delay(dht.getMinimumSamplingPeriod());
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  // Get data from the MQ-135 Gas Sensor
  int gas = analogRead(mq_135);
  String gas_status;
  if(gas > 305){ gas_status = "DANGER"; }else{ gas_status = "OK"; }
  // Set message texts.
  String _Temperature = "Temperature => " + String(temperature) + "*C";
  String _Humidity = "Humidity => " + String(humidity) + "%";
  String _Gas = "Gas Value => " + String(gas) + " ---- Status => " + gas_status;
  // Convert messages from strings to char arrays tp be able to send them via SIM808.
  String_to_Char(Temperature, message_size, _Temperature);
  String_to_Char(Humidity, message_size, _Humidity);
  String_to_Char(Gas, message_size, _Gas);
  
}

void read_SMS_Messages(){
 //******** Wait serial data *************
 if(sim808.readable()){
   sim808_read_buffer(gprsBuffer,32,DEFAULT_TIMEOUT);
   //Serial.print(gprsBuffer);
   //************** Detect the current state of SMS ************************
   if(NULL != (s = strstr(gprsBuffer,"+CMTI: \"SM\""))) { //SMS: $$+CMTI: "SM",24$$
      LCDA.CLEAR();
      char message[MESSAGE_LENGTH];
      int messageIndex = atoi(s+12);
      sim808.readSMS(messageIndex, message, MESSAGE_LENGTH);
      LCDA.DisplayString(0,0,"Recv. Message:", 14);
      LCDA.DisplayString(2,0,message,strlen(message));
      delay(3000);
      // Activate the requested command via SMS.
      activate_Commands(message);      
   }
   sim808_clean_buffer(gprsBuffer,32);
 }else{
  LCDA.DisplayString(0,0,"Waiting...",10);
  LCDA.DisplayString(1,0,"SMS Messages :)",15);
 }  
}

void activate_Commands(char command[]){
  if(strcmp(command, "Open Fan") == 0){
    LCDA.CLEAR();
    LCDA.DisplayString(0,0,"Fan",3);
    LCDA.DisplayString(1,0,"Activated!",10);
    digitalWrite(relay, LOW);
    delay(3000);
  }
  if(strcmp(command, "Close Fan") == 0){
    LCDA.CLEAR();
    LCDA.DisplayString(0,0,"Fan",3);
    LCDA.DisplayString(1,0,"Closed!",7);
    digitalWrite(relay, HIGH);
    delay(3000);
  }
  if(strcmp(command, "Temperature") == 0){
    // Get variables:
    get_Sensor_Variables();
    // Print
    LCDA.CLEAR();
    LCDA.DisplayString(0,0,"Temperature",11);
    LCDA.DisplayString(1,0,"Transferred!",12);
    // Send the temperature to the defined phone number as a feedback message.
    sim808.sendSMS(PHONE_NUMBER, Temperature);
    delay(3000);
  }
  if(strcmp(command, "Humidity") == 0){
    // Get variables:
    get_Sensor_Variables();
    // Print
    LCDA.CLEAR();
    LCDA.DisplayString(0,0,"Humidity",8);
    LCDA.DisplayString(1,0,"Transferred!",12);
    // Send the temperature to the defined phone number as a feedback message.
    sim808.sendSMS(PHONE_NUMBER, Humidity);
    delay(3000);
  }
  if(strcmp(command, "Gas") == 0){
    // Get variables:
    get_Sensor_Variables();
    // Print
    LCDA.CLEAR();
    LCDA.DisplayString(0,0,"Gas",3);
    LCDA.DisplayString(1,0,"Transferred!",12);
    // Send the temperature to the defined phone number as a feedback message.
    sim808.sendSMS(PHONE_NUMBER, Gas);
    delay(3000);
  }
  // Exit and Clear.
  LCDA.CLEAR();
}

void String_to_Char(char _convert[], int _size, String _String){
  for(int i=0;i<_size;i++){
    _convert[i] = _String[i];
  }
}
