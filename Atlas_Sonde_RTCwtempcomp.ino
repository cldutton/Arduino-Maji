
#include <SoftwareSerial.h> 		   		    
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "RTClib.h"

#define rxpin 2 					   //set the RX pin to pin 2                                                               
#define txpin 3                       	   	           //set the TX pin to pin 3  
#define OLED_RESET 7

Adafruit_SSD1306 display(OLED_RESET);
SoftwareSerial myserial(rxpin, txpin);   		   //enable the soft serial port    

String sensorstring = "";	                           //a string to hold the data from the Atlas Scientific product

short ec=0;                                                //this will tell the Arduino to open the EC channel
short d_o=1;                                               //this will tell the Arduino to open the D.O. channel
short orp=2;                                               //this will tell the Arduino to open the ORP channel
short ph=3;                                                //this will tell the Arduino to open the pH channel

float temp;                                                //where the final temperature data is stored

boolean sensor_stringcomplete=false;
short ack_channel_read=0;  

RTC_DS1307 RTC;

void setup(){
  
    Wire.begin();
    
    RTC.begin();  
  
    pinMode(4, OUTPUT);                            //used to control the S0 pin
    pinMode(5, OUTPUT);                            //used to control the S1 pin
    pinMode(6, OUTPUT);                            //temperature pin
    sensorstring.reserve(30);                      //set aside some bytes for receiving data
    myserial.begin(38400);                         //set up the soft serial to run at 38400              
    Serial.begin(9600);                            //set up the hardware serial port to run at 38400  
    
    RTC.adjust(DateTime(__DATE__, __TIME__));      //sets the RTC to the date & time this sketch was compiled
     
    display.begin(SSD1306_SWITCHCAPVCC, 0x3D);     //initialize with the I2C addr 0x3D (for the 128x64)
     
    display.display();
    
    display.clearDisplay();
    
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println(F(" Water Quality Logger"));
}
            
void loop() {                                   	 
      
      display.display();
      
      while(ack_channel_read==0){
            Open_channel(ec);                       //we call the sub open channel. The sub needs to pass a val to indicate what channel to open, ec is set to 0. So, we are telling the function to open channel 0  
	    ack_channel_read=read_channel(ec);
            }
      
      while(ack_channel_read==1){
           Open_channel(d_o);                      
	   read_channel(d_o);
          }
   
      while(ack_channel_read==2){
           Open_channel(orp);                      
	   read_channel(orp);
          }
          
      while(ack_channel_read==3){
           Open_channel(ph);                      
	   read_channel(ph);
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

       if(channel==0){                                                              //the DO probe needs temperature compensation, so we call temp first
         
           temp = read_temp();                                                      //call the function “read_temp” and return the temperature in C°
           
           myserial.print(temp);                                                    //then send that info to the DO probe to get the temp compensated value
           myserial.print(",0");  
           myserial.write(13);

           delay(1100);                                                              //let 1.1 sec pass
      
           while (myserial.available()) {                                            //while a char is holding in the serial buffer
               char inchar = (char)myserial.read();                                  //get the new char
               sensorstring += inchar;                                               //add it to the sensorString
               if (inchar == '\r') {sensor_stringcomplete = true;}                   //if the incoming character is a <CR>, set the flag
           }
             
          if (sensor_stringcomplete){                                                //if a string from the Atlas Scientific product has been received in its entirety

            Serial.print(F("DO: "));
            display.setCursor(0,17);
            display.print(F("DO: "));
            display.setCursor(30,17);
            display.drawRect(30, 17, 90, 8, BLACK);                                  //draw a black box to clear the old DO info
            display.fillRect(30, 17, 90, 8, BLACK);
            display.setCursor(30,17);
            ack_channel_read=1;
            Serial.println(sensorstring);                                             
            display.println(sensorstring);                                           //write the new DO values onto the screen
            display.setCursor(0,25);
            display.print(F("Temp: "));
            display.drawRect(30, 25, 90, 8, BLACK);                                  //draw a black box to clear the old temp info
            display.fillRect(30, 25, 90, 8, BLACK);

            display.setCursor(30,25);
            display.print(temp);                                                     //write the new temp info onto the screen
            sensorstring = ""; 
          }
          DateTime now = RTC.now();
    
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
          
          display.setCursor(12,57);
          display.drawRect(12, 57, 118, 8, BLACK);
          display.fillRect(12, 57, 118, 8, BLACK);
          display.setCursor(12,57);
          display.print(now.year(), DEC);
          display.print('/');
          display.print(now.month(), DEC);
          display.print('/');
          display.print(now.day(), DEC);
          display.print(' ');
          display.print(now.hour(), DEC);
          display.print(':');
          display.print(now.minute(), DEC);
          display.print(':');
          display.print(now.second(), DEC);
          
          display.display();
          //clear the string:
       sensor_stringcomplete = false; 
          return ack_channel_read;
       }
       
     if (channel==1 || channel==2 || channel==3){                              //the other three probes do not need temperature compensation
          
     myserial.print("r");   
     myserial.write(13);

      delay(1100);                                                             //let 1.1 sec pass
      
      while (myserial.available()) {                                           //while a char is holding in the serial buffer
         char inchar = (char)myserial.read();                                  //get the new char
         sensorstring += inchar;                                               //add it to the sensorString
         if (inchar == '\r') {sensor_stringcomplete = true;}                   //if the incoming character is a <CR>, set the flag
         }
       
       if (sensor_stringcomplete){                                             //if a string from the Atlas Scientific product has been received in its entirety
     
       if(channel==1){                                                         //ORP
         Serial.print(F("ORP: ")); 
         display.setCursor(0,33);
         display.print(F("ORP: "));
         display.setCursor(30,33);
         display.drawRect(30, 33, 90, 8, BLACK);                               //draw a black box to clear the old ORP reading
         display.fillRect(30, 33, 90, 8, BLACK);
         ack_channel_read=2;
         Serial.print(sensorstring);                                           
         display.println(sensorstring);                                        //write the new ORP value onto the screen
         Serial.println();
         sensorstring = ""; 
         }
       
       if(channel==2){
          Serial.print(F("pH: "));                                             //pH
          display.setCursor(0,41);
          display.print(F("pH: "));
          display.setCursor(30,41);
          display.drawRect(30, 41, 90, 8, BLACK);                              //draw a black box to clear the old pH reading
          display.fillRect(30, 41, 90, 8, BLACK);
          ack_channel_read=3;
          Serial.print(sensorstring);                                             
          display.println(sensorstring);                                       //write the new pH value onto the screen
          Serial.println();
          sensorstring = ""; 
          }
       
       if(channel==3){
          Serial.print(F("EC: "));                                              //conductivity
          display.setCursor(0,49);
          display.print(F("EC: ")); 
          display.setCursor(30,49);
          display.drawRect(30, 49, 90, 8, BLACK);                              //draw a black box to erase the old conductivity reading
          display.fillRect(30, 49, 90, 8, BLACK);
          ack_channel_read=4;
          Serial.print(sensorstring);                                          
          display.println(sensorstring);                                        //write the new conductivity value onto the screen
          Serial.println();
          sensorstring = ""; 
          }  
       }
           
          DateTime now = RTC.now();                                             //get the current date and time then write refresh it on the screen
    
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
          
          display.setCursor(12,57);
          display.drawRect(12, 57, 118, 8, BLACK);
          display.fillRect(12, 57, 118, 8, BLACK);
          display.setCursor(12,57);
          display.print(now.year(), DEC);
          display.print('/');
          display.print(now.month(), DEC);
          display.print('/');
          display.print(now.day(), DEC);
          display.print(' ');
          display.print(now.hour(), DEC);
          display.print(':');
          display.print(now.minute(), DEC);
          display.print(':');
          display.print(now.second(), DEC);
          
          display.display();
          //clear the string:
          sensor_stringcomplete = false;     
           
          return ack_channel_read;
     }    
  }  
  
float read_temp(void){   //the read temperature function
  float v_out;             //voltage output from temp sensor 
  float temp;              //the final temperature is stored here
  digitalWrite(A0, LOW);   //set pull-up on analog pin
  digitalWrite(6, HIGH);   //set pin 2 high, this will turn on temp sensor
  delay(2);                //wait 2 ms for temp to stabilize
  v_out = analogRead(0);   //read the input pin
  digitalWrite(6, LOW);    //set pin 2 low, this will turn off temp sensor
  v_out*=.0048;            //convert ADC points to volts (we are using .0048 because this device is running at 5 volts)
  v_out*=1000;             //convert volts to millivolts
  temp= 0.0512 * v_out -20.5128; //the equation from millivolts to temperature
  return temp;             //send back the temp
}
