/*
This is code is for a non-contact bridge mountable water level logger.  Information
is transmitted to Twitter every 30 minutes if the water level is different than the
previous reading.  

Most of this code is from the Maxbotix site, the Adafruit site, the Seeedstudio site
and the Arduino site.  Much thanks goes to all the generous people out there sharing 
their code. 

I'm sure this code could be drastically improved.  Hopefully, someone will, someday.  
*/

#include <SD.h>                    //library for the SD Card
#include <Wire.h>                  //library to communicate with I2C devices, the RTC
#include <RTClib.h>                //library for the real-time clock - RTC
#include <Time.h>                  //library to help keep time
#include <TimeAlarms.h>            //library for the timer
#include <SoftwareSerial.h>        //library to communicate with the GPRS modem over software serial
#include <String.h>                //library to help deal with strings

#define SYNC_INTERVAL 1000 // mills between calls to flush() - to write data to the card
uint32_t syncTime = 0; // time of last sync()

SoftwareSerial mySerial(7,8);  // pins for the gprs modem

// gprs code from http://www.seeedstudio.com/wiki/GPRS_Shield_V2.0
// SD card and data logging code from http://learn.adafruit.com/adafruit-data-logger-shield

RTC_DS1307 RTC;                        //define the Real Time Clock object

int sensorPin = A0;                    //this is the pin the sensor data comes in through
int arraysize = 5;                     //quantity of values to find the median (sample size). Needs to be an odd number
int rangevalue[] = {0, 0, 0, 0, 0};    //declare an array to store the samples. not necessary to zero the array values here, it just makes the code clearer
int previouswaterlevel = 0;            //setting the previous waterlevel as 0

const int chipSelect = 10;             //for the data logging shield, we use digital pin 10 for the SD cs line

const int analogInPin = A1;            //analog input pin that the VBAT pin is attached to 
 
int BatteryValue = 0;                  //value read from the VBAT pin
float outputValue = 0;                 //variable for voltage calculation 
 
File logfile;                          //the logging file
 
void error(char *str)                  //generic error routine
{
  Serial.print("error: ");
  Serial.println(str);
  
  while(1);
}

time_t syncProvider()                  //function that does the syncing between the internal clock and the RTC
{
  return RTC.now().unixtime();
}

void setup(){
  Serial.begin(19200);                 //set up the hardware serial port to run at 38400
  mySerial.begin(19200);               //set up the software serial port for the gprs modem
  Wire.begin();                        //start the wire library
  
  RTC.begin();                         //start the RTC
  
  setSyncProvider(syncProvider);              //syncs the internal Arduino clock with the RTC

  Alarm.timerRepeat(1800, Repeats);          //timer for every 30 minutes (1800), 900 seconds for 15 minutes, 60 for one minute
 
  pinMode(10, OUTPUT);                      //Pin used for the SD Card
 
  if (!SD.begin(chipSelect)) {               //check to see if the SD Card is present
    error("Card failed, or not present");
  }
  
  Serial.println("Welcome to the Water Level Logger.");
  
  char filename[] = "LOGGER00.CSV";        //create a new file everytime the unit restarts
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      logfile = SD.open(filename, FILE_WRITE);              //only open a new file if it doesn't exist
      break;
    }
  }
  
  if (! logfile) {
    error("could not create file");
  }
  
  Serial.print("Logging to: ");
  Serial.println(filename);
  
  logfile.println("DateTime, Water Level (cm), Battery Volts");    // print this as first line of SD saved data
  Serial.println("DateTime, Water Level (cm), Battery Volts");     // print this to the PC
  
}

void loop() {
   Alarm.delay(1000);                  // need this here for the alarms and timers to be called
}

void Repeats(){
    for(int i = 0; i < arraysize; i++)
   {                                                    //array pointers go from 0 to 4
      // http://www.maxbotix.com/documents/HRXL-MaxSonar-WR_Datasheet.pdf
       rangevalue[i] = analogRead(sensorPin);
       delay(10);  //wait between analog samples
    }  
  
    int midpoint = arraysize/2;    //midpoint of the array is the medain value in a sorted array
      
    int depthvalue = 700 - rangevalue[midpoint];    //700 is the adjustment value for the depth for this site
  
    delay(1000); // delay to let the machine relax a bit
  
    // Code for getting the battery voltage
    BatteryValue = analogRead(analogInPin);            // read the analog in value:   
    outputValue = (float(BatteryValue)*5)/1023*2;    // Calculate the battery voltage value
     
    DateTime now;      // create a Datetime object
 
    now = RTC.now();    // pull the time from the RTC and put it in the new DateTime object called "now"
        
    // print the date time data to the serial connection - PC   
    Serial.print('"');
    Serial.print(now.year(), DEC);
    Serial.print("/");
    Serial.print(now.month(), DEC);
    Serial.print("/");
    Serial.print(now.day(), DEC);
    Serial.print(" ");
    Serial.print(now.hour(), DEC);
    Serial.print(":");
    Serial.print(now.minute(), DEC);
    Serial.print(":");
    Serial.print(now.second(), DEC);
    Serial.print('"');
    Serial.print(", ");
    Serial.print(depthvalue);     //print the depth data
    Serial.print(" cm, ");
    Serial.print(outputValue);
    Serial.print(" volts");
    Serial.println("");   
  
    // save the data to the SD card
    logfile.print('"');
    logfile.print(now.year(), DEC);
    logfile.print("/");
    logfile.print(now.month(), DEC);
    logfile.print("/");
    logfile.print(now.day(), DEC);
    logfile.print(" ");
    logfile.print(now.hour(), DEC);
    logfile.print(":");
    logfile.print(now.minute(), DEC);
    logfile.print(":");
    logfile.print(now.second(), DEC);
    logfile.print('"');
    logfile.print(", ");
    logfile.print(depthvalue);     //print the depth data
    logfile.print(", ");
    logfile.print(outputValue);
    logfile.println(""); 
  
    if (rangevalue[midpoint] != previouswaterlevel)   // check to see if the depth reading is the same as before, if so, don't send it via GPRS
     {
          // turn modem on 
        pinMode(9, OUTPUT);
        digitalWrite(9,LOW);
        delay(100);
        digitalWrite(9,HIGH);
        delay(500);
        digitalWrite(9,LOW);
        delay(100);
    
        delay(10000);                                      //wait ten seconds to make sure it has time to connect to the network
      
        mySerial.print("AT+CMGF=1\r");                     //line for sms mode
        delay(100);
        mySerial.println("AT + CMGS = \"8988\"");          //number to send the text to with country code, 8988 is the Twitter short code for Kenya
        delay(100);
        mySerial.print(depthvalue);                        //the content of the message, only the depth
        mySerial.print(" #thingspeak #nmbriverdepth");     //followed by the thingspeak hashtags
        delay(100);
        mySerial.println((char)26);                        //the ASCII code of the ctrl+z is 26, which closes the sms
        delay(10000);                                      //10 second delay
        mySerial.println();
      
          // turn modem off
        digitalWrite(9,LOW);
        delay(1000);
        digitalWrite(9,HIGH);
        delay(2000);
        digitalWrite(9,LOW);
        delay(3000);
     }
  
  previouswaterlevel = rangevalue[midpoint];  //save the most current water level reading to compare next time

  logfile.flush();
} 

void isort(int *a, int n)    //  *a is an array pointer function, sort function for getting the median value from the Maxbotix sensor, from Arduino website
{
  for (int i = 1; i < n; ++i)
  {
    int j = a[i];
    int k;
    for (k = i - 1; (k >= 0) && (j < a[k]); k--)
    {
      a[k + 1] = a[k];
    }
    a[k + 1] = j;
  }
}
