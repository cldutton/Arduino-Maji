#include <SoftwareSerial.h> 		   		   //for communicating with the probes
#include <Wire.h>                                          //for communicating with I2C devices - the RTC
#include "SD.h"                                            //library for the SD Card
#include "RTClib.h"                                        //library for the real-time clock(RTC)
#include <Time.h>

#define rxpin 2 					   //set the RX pin to pin 2 for the multiplexer                                                              
#define txpin 3                       	   	           //set the TX pin to pin 3 for the multiplexer

#define WAIT_TO_START    0 // Wait for serial input in setup()

SoftwareSerial myserial(rxpin, txpin);   		   //enable the soft serial port    

String sensorstring = "";	        //a string to hold the data from the Atlas Scientific product
String dosensorstring = "";
String logstring = "";

float temp;                            //final temp data stored here

boolean sensor_stringcomplete=false;   //used to tell when you have received all the characters from the sensors
short ack_channel_read=0;              //used to determine which channel from the multiplexer you are going to read from

RTC_DS1307 RTC;                        //defines the real-time clock

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;
 
// the logging file
File logfile;
 
void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  
  while(1);
}

time_t syncProvider()     // function that does the syncing between the internal clock and the RTC
{
  return RTC.now().unixtime();
}

void setup(){
  
    Wire.begin();                           // starts the Wire library transmission stuff
    
    RTC.begin();                            //starts the RTC
  
    pinMode(4, OUTPUT);                      //used to control the S0 pin
    pinMode(5, OUTPUT);                      //used to control the S1 pin
    pinMode(6, OUTPUT);                      //temperature pin
    sensorstring.reserve(30);                //set aside some bytes for receiving data
    myserial.begin(38400);                   //set up the soft serial to run at 38400              
    Serial.begin(38400);                     //set up the hardware serial port to run at 38400  
    
    RTC.adjust(DateTime(__DATE__, __TIME__));
    
    setSyncProvider(syncProvider);              // syncs the internal Arduino clock with the RTC
     
    pinMode(10, OUTPUT);
// see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    error("Card failed, or not present");
  }
  Serial.println("Welcome to the Water Quality Logger.");
  
  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }
  
  if (! logfile) {
    error("couldnt create file");
  }
  
  Serial.print("Logging to: ");
  Serial.println(filename);

}
            
void loop() {                                   	 
      
      while(ack_channel_read==0){
            Open_channel(0);                       //we call the sub open channel. The sub needs to pass a val to indicate what channel to open, ec is set to 0. So, we are telling the function to open channel 0  
	    ack_channel_read=read_channel(0);
            }
      
      while(ack_channel_read==1){
           Open_channel(1);                      
	   read_channel(1);
          }
   
      while(ack_channel_read==2){
           Open_channel(2);                      
	   read_channel(2);
          }
          
      while(ack_channel_read==3){
           Open_channel(3);                      
	   read_channel(3);
          }
 
     if(ack_channel_read==4){ack_channel_read=0;}
      
}

void Open_channel(short channel){

    switch (channel) {
    
      case 0:                                  //open channel Y0 
        digitalWrite(4, LOW);                  //S0=0    
        digitalWrite(5, LOW);                  //S1=0
      break;
      
      case 1:                                  //open channel Y1 
       digitalWrite(4, HIGH);                  //S0=1    
       digitalWrite(5, LOW);                   //S1=0
      break;
      
      case 2:                                  //open channel Y2    
       digitalWrite(4, LOW);                   //S0=0    
       digitalWrite(5, HIGH);                  //S1=1
      break;
      
      case 3:                                  //open channel Y3 
       digitalWrite(4, HIGH);                  //S0=1    
       digitalWrite(5, HIGH);                  //S1=1
      break;
    }     
	
return; 
 }   
            
 short read_channel(short channel){

       if(channel==0){
         
           temp = read_temp();       //call the function “read_temp” and return the temperature in C°
           
           myserial.print(temp);     //send the temp value to the DO unit for temperature compensated DO reading
           myserial.print(",0");     //finish the command to the DO unit by telling it no salinity compensation
           myserial.write(13);        //terminate the command string to the DO unit

           delay(1100);                            //let 1.1 sec at a minimum pass...set to 2 now 
      
           while (myserial.available()) {                                               //while a char is holding in the serial buffer
               char inchar = (char)myserial.read();                                  //get the new char
               dosensorstring += inchar;                                               //add it to the sensorString
               if (inchar == '\r') {sensor_stringcomplete = true;}                   //if the incoming character is a <CR>, set the flag
           }
             
          if (sensor_stringcomplete){                                                 //if a string from the Atlas Scientific product has been received in its entirety
            
            ack_channel_read=1;
            
            DateTime now = RTC.now();
            
            logstring = "";
            logstring += "DO: ";
            logstring += String(dosensorstring);
            logstring += ", ";
            logstring += String(now.year()); 
 
           Serial.print(logstring);           
   
            
            logfile.print("DO: ");
            logfile.print(dosensorstring);                //print the data to the PC
            logfile.print(", ");
            logfile.print("DATETIME: ");
            logfile.print(now.year(), DEC);
            logfile.print('/');
            logfile.print(now.month(), DEC);
            logfile.print('/');
            logfile.print(now.day(), DEC);
            logfile.print(' ');
            logfile.print(now.hour(), DEC);
            logfile.print(':');
            logfile.print(now.minute(), DEC);
            logfile.print(':');
            logfile.print(now.second(), DEC);
            logfile.println();
            
            Serial.print("DO: ");
            Serial.print(dosensorstring);                //print the data to the PC
            
            Serial.print(", ");
            Serial.print("DATETIME: ");
            Serial.print(now.year(), DEC);
            Serial.print('/');
            Serial.print(now.month(), DEC);
            Serial.print('/');
            Serial.print(now.day(), DEC);
            Serial.print(' ');
            Serial.print(now.hour(), DEC);
            Serial.print(':');
            Serial.print(now.minute(), DEC);
            Serial.print(':');
            Serial.print(now.second(), DEC);
            Serial.println();
            
            logfile.print("Temp: ");
            logfile.print(temp);                      //print temp data to PC
            logfile.print(", ");
            logfile.print("DATETIME: ");
            logfile.print(now.year(), DEC);
            logfile.print('/');
            logfile.print(now.month(), DEC);
            logfile.print('/');
            logfile.print(now.day(), DEC);
            logfile.print(' ');
            logfile.print(now.hour(), DEC);
            logfile.print(':');
            logfile.print(now.minute(), DEC);
            logfile.print(':');
            logfile.print(now.second(), DEC);
            logfile.println();
            
            Serial.print("Temp: ");
            Serial.print(temp);                      //print temp data to PC
            
            Serial.print(", ");
            Serial.print("DATETIME: ");
            Serial.print(now.year(), DEC);
            Serial.print('/');
            Serial.print(now.month(), DEC);
            Serial.print('/');
            Serial.print(now.day(), DEC);
            Serial.print(' ');
            Serial.print(now.hour(), DEC);
            Serial.print(':');
            Serial.print(now.minute(), DEC);
            Serial.print(':');
            Serial.print(now.second(), DEC);
            Serial.println();
            
            dosensorstring = " ";                         //clear the sensorstring
                      
            logfile.flush();
            
            sensor_stringcomplete = false;           //set this as false so it can move onto the next channel to get data
  
          }
          
          return ack_channel_read;
       }
       
     if (channel==1 || channel==2 || channel==3){              //any of the other probes do not need temp compensation
     
               DateTime now = RTC.now();
          
     myserial.print("r");   //send command to the probe to tell it to start sending data
     myserial.write(13);    //terminate command to probe

      delay(1100);                            //let 1.1 sec pass, 2 seconds
      
      while (myserial.available()) {                                               //while a char is holding in the serial buffer
         char inchar = (char)myserial.read();                                  //get the new char
         sensorstring += inchar;                                               //add it to the sensorString
         if (inchar == '\r') {sensor_stringcomplete = true;}                   //if the incoming character is a <CR>, set the flag
         }
       
       if (sensor_stringcomplete){                                                 //if a string from the Atlas Scientific product has been received in its entirety
     
         if(channel==1){
         
           ack_channel_read=2;
           
           now = RTC.now();
           logfile.print("ORP: ");
           logfile.print(sensorstring);
          logfile.print(", ");
          logfile.print("DATETIME: ");
          logfile.print(now.year(), DEC);
          logfile.print('/');
          logfile.print(now.month(), DEC);
          logfile.print('/');
          logfile.print(now.day(), DEC);
          logfile.print(' ');
          logfile.print(now.hour(), DEC);
          logfile.print(':');
          logfile.print(now.minute(), DEC);
          logfile.print(':');
          logfile.print(now.second(), DEC);
          logfile.println();
          
           Serial.print("ORP: ");
           Serial.print(sensorstring);
           sensorstring = " "; 
           
           logfile.flush();
         }
       
         if(channel==2){
          
            ack_channel_read=3;
          
          now = RTC.now();
          
            logfile.print("pH: ");
            logfile.print(sensorstring); 
          logfile.print(", ");
          logfile.print("DATETIME: ");
          logfile.print(now.year(), DEC);
          logfile.print('/');
          logfile.print(now.month(), DEC);
          logfile.print('/');
          logfile.print(now.day(), DEC);
          logfile.print(' ');
          logfile.print(now.hour(), DEC);
          logfile.print(':');
          logfile.print(now.minute(), DEC);
          logfile.print(':');
          logfile.print(now.second(), DEC);
          logfile.println();
          
            Serial.print("pH: ");
            Serial.print(sensorstring);   
            sensorstring = " "; 
            
            logfile.flush();
          }
       
         if(channel==3){
            
            ack_channel_read=4;
            
            now = RTC.now();
          
            logfile.print("EC: ");
            logfile.print(sensorstring);
          logfile.print(", ");
          logfile.print("DATETIME: ");
          logfile.print(now.year(), DEC);
          logfile.print('/');
          logfile.print(now.month(), DEC);
          logfile.print('/');
          logfile.print(now.day(), DEC);
          logfile.print(' ');
          logfile.print(now.hour(), DEC);
          logfile.print(':');
          logfile.print(now.minute(), DEC);
          logfile.print(':');
          logfile.print(now.second(), DEC);
          logfile.println();
          
            Serial.print("EC: ");
            Serial.print(sensorstring);  
            sensorstring = " "; 
            //channel three is the last channel to check.  get the data and time then print to the PC as a time stamp
            
            logfile.flush();
          }
          
            Serial.print(", ");
            Serial.print("DATETIME: ");
            Serial.print(now.year(), DEC);
            Serial.print('/');
            Serial.print(now.month(), DEC);
            Serial.print('/');
            Serial.print(now.day(), DEC);
            Serial.print(' ');
            Serial.print(now.hour(), DEC);
            Serial.print(':');
            Serial.print(now.minute(), DEC);
            Serial.print(':');
            Serial.print(now.second(), DEC);
            Serial.println();
          
          sensor_stringcomplete = false;     
         }
          return ack_channel_read;
     }    
    
  }  
                                          
float read_temp(void){                   //this is the read temperature function provided by Atlas Scientific
      float v_out;                       //voltage output from temp sensor 
      float temp;                        //the final temperature is stored here
      digitalWrite(A0, LOW);             //set pull-up on analog pin
      digitalWrite(6, HIGH);             //set pin 2 high, this will turn on temp sensor
      delay(2);                          //wait 2 ms for temp to stabilize
      v_out = analogRead(0);             //read the input pin
      digitalWrite(6, LOW);              //set pin 2 low, this will turn off temp sensor
      v_out*=.0048;                      //convert ADC points to volts (we are using .0048 because this device is running at 5 volts)
      v_out*=1000;                       //convert volts to millivolts
      temp= 0.0512 * v_out -20.5128;     //the equation from millivolts to temperature
      return temp;                       //send back the temp
}
