/*
This is code to use the four main Atlas Scientific probes as a handheld water quality device, printing the information to a small 
OLED screen.  Most of this code is from the Atlas Scientific site or from the Adafruit site. 

I'm sure this code could be drastically improved.  Hopefully, someone will, someday.  
*/
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
String dosensorstring = "";

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
    Serial.begin(38400);                            //set up the hardware serial port to run at 38400  
    
    
//    RTC.adjust(DateTime(__DATE__, __TIME__));      //sets the RTC to the date & time this sketch was compiled
     
    display.begin(SSD1306_SWITCHCAPVCC, 0x3D);     //initialize with the I2C addr 0x3D (for the 128x64)
     
    display.display();                             //this tells the display to start working
    
    display.clearDisplay();                        //make sure there is nothing in the display buffer
    
    display.setTextSize(1);                        //set the text size to size "1"
    display.setTextColor(WHITE);                   //set the text color to white
    display.setCursor(0,0);                        //set the cursor to start in the top left hand side of the screen
    display.println(F(" Water Quality Logger"));   //now print this text on the top line
}
            
void loop() {                                   	 
      
      display.display();                            //make sure the display is on
      
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

 void Open_channel(short channel){            //this is the function for choosing which probe you would like to talk to using the multiplexer (the yellow board)                                  

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
           
           delay(100);
           
           myserial.print(temp);                                                    //then send that info to the DO probe to get the temp compensated value
           myserial.print(",0");  
           myserial.write(13);

           delay(1100);                                                              //let 1.1 sec pass
      
           while (myserial.available()) {                                            //while a char is holding in the serial buffer
               char inchar = (char)myserial.read();                                  //get the new char
               dosensorstring += inchar;                                               //add it to the sensorString
               if (inchar == '\r') {sensor_stringcomplete = true;}                   //if the incoming character is a <CR>, set the flag
           }
             
          if (sensor_stringcomplete){                                                //if a string from the Atlas Scientific product has been received in its entirety

            Serial.print(F("DO: "));
            display.setCursor(30,17);
            display.drawRect(0, 17, 128, 8, BLACK);                                  //draw a black box to clear the old DO info
            display.fillRect(0, 17, 128, 8, BLACK);
            display.setCursor(0,17);
            display.print(F("DO: "));
            display.setCursor(30,17);
            Serial.println(dosensorstring);                                             
            display.println(dosensorstring);                                           //write the new DO values onto the screen

            display.drawRect(0, 25, 128, 8, BLACK);                                  //draw a black box to clear the old temp info
            display.fillRect(0, 25, 128, 8, BLACK);
            display.setCursor(0,25);
            display.print(F("Temp: "));

            display.setCursor(30,25);
            display.print(temp);                                                     //write the new temp info onto the screen
            
            sensor_stringcomplete = false;                                              //this resets string counter
            dosensorstring = ""; 
            
            DateTime now = RTC.now();                                                  //get the current date and time
    
          Serial.print(now.year(), DEC);                                            //then print it on the computer
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
          
          display.setCursor(12,57);                                                  //set the cursor where you want to print the date on the oled
          display.drawRect(12, 57, 128, 8, BLACK);                                   //first, print a black rectangle in that area
          display.fillRect(12, 57, 128, 8, BLACK);                                   //then fill it....this will essentially clear that space so you can print the new date
          display.setCursor(12,57);                                                  //reset the cursor so the date spot
          display.print(now.year(), DEC);                                            //now, start printing the date onto the oled
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
          
          ack_channel_read=1;
            
          }

          return ack_channel_read;

       }
       
     if (channel==1 || channel==2 || channel==3){                              //the other three probes do not need temperature compensation
          
     myserial.print(F("r"));                                                      //the command to tell the atlas probes you want some info
     myserial.write(13);                                                       //followed by the 13 character

      delay(1100);                                                             //let 1.1 sec pass
      
      while (myserial.available()) {                                           //while a char is holding in the serial buffer
         char inchar = (char)myserial.read();                                  //get the new char
         sensorstring += inchar;                                               //add it to the sensorString
         if (inchar == '\r') {sensor_stringcomplete = true;}                   //if the incoming character is a <CR>, set the flag
         }
       
       if (sensor_stringcomplete){
     
       if(channel==1){                                                         //ORP
         Serial.print(F("ORP: "));
         display.setCursor(30,33);
         display.drawRect(0, 33, 128, 8, BLACK);                               //draw a black box to clear the old ORP reading
         display.fillRect(0, 33, 128, 8, BLACK);
         display.setCursor(0,33);
         display.print(F("ORP: "));
                  
         Serial.print(sensorstring);                                           
         display.println(sensorstring);                                        //write the new ORP value onto the screen
         Serial.println();
         sensorstring = ""; 
         
         display.display();
         
         ack_channel_read=2;
         
         DateTime now = RTC.now();                                                  //get the current date and time
    
          Serial.print(now.year(), DEC);                                            //then print it on the computer
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
          
          display.setCursor(12,57);                                                  //set the cursor where you want to print the date on the oled
          display.drawRect(12, 57, 128, 8, BLACK);                                   //first, print a black rectangle in that area
          display.fillRect(12, 57, 128, 8, BLACK);                                   //then fill it....this will essentially clear that space so you can print the new date
          display.setCursor(12,57);                                                  //reset the cursor so the date spot
          display.print(now.year(), DEC);                                            //now, start printing the date onto the oled
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

         }
       
       if(channel==2){
          Serial.print(F("pH: "));                                             //pH
          display.setCursor(30,41);
          display.drawRect(0, 41, 128, 8, BLACK);                              //draw a black box to clear the old pH reading
          display.fillRect(0, 41, 128, 8, BLACK);
          display.setCursor(0,41);
          display.print(F("pH: "));
          
          Serial.print(sensorstring);                                             
          display.println(sensorstring);                                       //write the new pH value onto the screen
          Serial.println();
          sensorstring = ""; 
          
          display.display();
          
          ack_channel_read=3;
          
          DateTime now = RTC.now();                                                  //get the current date and time
    
          Serial.print(now.year(), DEC);                                            //then print it on the computer
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
          
          display.setCursor(12,57);                                                  //set the cursor where you want to print the date on the oled
          display.drawRect(12, 57, 128, 8, BLACK);                                   //first, print a black rectangle in that area
          display.fillRect(12, 57, 128, 8, BLACK);                                   //then fill it....this will essentially clear that space so you can print the new date
          display.setCursor(12,57);                                                  //reset the cursor so the date spot
          display.print(now.year(), DEC);                                            //now, start printing the date onto the oled
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
          }
       
       if(channel==3){
          Serial.print(F("EC: "));                                              //conductivity
          display.setCursor(30,49);
          display.drawRect(0, 49, 128, 8, BLACK);                              //draw a black box to erase the old conductivity reading
          display.fillRect(0, 49, 128, 8, BLACK);
          display.setCursor(0,49);
          display.print(F("EC: ")); 
                    
          Serial.print(sensorstring);                                          
          display.println(sensorstring);                                        //write the new conductivity value onto the screen
          Serial.println();
          sensorstring = ""; 
          
          display.display();
          
          ack_channel_read=4;
          
          DateTime now = RTC.now();                                                  //get the current date and time
    
          Serial.print(now.year(), DEC);                                            //then print it on the computer
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
          
          display.setCursor(12,57);                                                  //set the cursor where you want to print the date on the oled
          display.drawRect(12, 57, 128, 8, BLACK);                                   //first, print a black rectangle in that area
          display.fillRect(12, 57, 128, 8, BLACK);                                   //then fill it....this will essentially clear that space so you can print the new date
          display.setCursor(12,57);                                                  //reset the cursor so the date spot
          display.print(now.year(), DEC);                                            //now, start printing the date onto the oled
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
          }
          sensor_stringcomplete = false;     
       }
     }   
    return ack_channel_read; 
  }  
  
float read_temp(void){                 //the read temperature function
  float v_out;                         //voltage output from temp sensor 
  float temp;                          //the final temperature is stored here
  digitalWrite(A0, LOW);               //set pull-up on analog pin
  digitalWrite(6, HIGH);               //set pin 2 high, this will turn on temp sensor
  delay(2);                            //wait 2 ms for temp to stabilize
  v_out = analogRead(0);               //read the input pin
  digitalWrite(6, LOW);                //set pin 2 low, this will turn off temp sensor
  v_out*=.0048;                        //convert ADC points to volts (we are using .0048 because this device is running at 5 volts)
  v_out*=1000;                         //convert volts to millivolts
  temp= 0.0512 * v_out -20.5128;       //the equation from millivolts to temperature
  return temp;                         //send back the temp
}
