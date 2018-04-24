/**
Modifications made by Newport High School (Alan Chu, Michael Li, Chloe Choi) for 2017 mission.
Used with perimission. Original license is still applicable.
*/

/*-------------------------------------------------------------------------------------------
The MIT License (MIT)

Copyright (c) 2017 Dan Ruder

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-------------------------------------------------------------------------------------------
FlightController.ino
Written by Dan Ruder

This program is designed for the Bellevue Satellite Club flight controller, which uses the
Adafruit Feather M0 Adalogger https://www.adafruit.com/products/2796.


-------------------------------------------------------------------------------------------
 Design Notes:
 -------------
 This program uses a loop within loop() so that loop() runs only once -- like C++ main.  
 Doing so solves problems where variables that need to be visible to both setup() and loop()
 need to be declared as global, but if you declare a class as global, its constructor gets 
 called before the Arduino library gets initialized by an interal function named init().
 An example of this problem is if a class constructor tries to use a Wire library function
 to initialize an i2c device.  If you declare a global variable of such a class, it will
 brick the Arduino and you will have to do a recovery reset to get it to work again.
 
 Bottom line, we can write better C++ this way.
-------------------------------------------------------------------------------------------*/

#define DEBUG 1

#include <dbghelp.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Light_TSL2561.h>
#include <Pressure_HSC_SSC.h>
#include <RTClib.h>
#include <stdio.h>
#include <Geiger_MightyOhm.h>
#include <Gyro_L3GD20H.h>
#include <Accel_LSM303.h>



// Global hardware definitions
constexpr int HARDWARE_SETTLE_TIME  = 4000;

constexpr int START_PIN     =  6;  // Reads HIGH when removed, LOW when inserted
constexpr int LED_PIN       =  8;  // Green LED on Feather M0
constexpr int SD_READER_PIN =  4;  // SD Card reader pin Feather M0 Adalogger
constexpr int SD_PRESENT_PIN = 7;  // micro SD card present pin; reads HIGH when inserted, LOW when removed


// Helper functions
void BlinkMorseCode (const char *pattern);
void print_date (DateTime &dt, Stream &s = Serial);
void print_time (DateTime &dt, Stream &s = Serial);


void loop()
{
   /////////////////////////////////////////////////////////////////////////////////////////
   // One-time startup initialization code here

   //---------------------------------------------------------------------------------------
   // Initialize hardware
   
   delay (HARDWARE_SETTLE_TIME);

   pinMode (START_PIN,      INPUT);
   pinMode (LED_PIN,        OUTPUT);
   pinMode (SD_READER_PIN,  OUTPUT);
   pinMode (SD_PRESENT_PIN, INPUT_PULLUP);

   // Initialize SPI slaves to not selected
   digitalWrite (SD_READER_PIN, HIGH);

   // Initialize I2C and serial ports
   Wire.begin();
   Serial.begin (9600);


   //---------------------------------------------------------------------------------------
   // Initialize the Real-time clock
   
   RTC_DS3231 rtc;
   rtc.begin();

   DebugSprintfIf(rtc.lostPower(), "RTC lost power, please run Set_RTC_Time to reset the time and date\n");


   //---------------------------------------------------------------------------------------
   // Enable the SD card reader - requires preformatted SD card to be inserted in SD reader.
   DebugSprintf ("Initializing SD reader\n");
   
   while (!SD.begin (SD_READER_PIN))
   {
      // This is a fatal error; alert the user until fixed.
      BlinkMorseCode ("---"); 
      DebugSprintf ("SD reader failed to initialize; is card missing?\n");
      delay (1000);
   }
   DebugSprintf ("SD reader initialized, card present\n");

   // If the data files already exist, opening them again will just append data.  We don't 
   // want this, so delete the files.
   if (SD.exists ("sensor.txt")) { SD.remove ("sensor.txt"); }
   if (SD.exists ("geiger.txt")) { SD.remove ("geiger.txt"); }
   if (SD.exists ("gyro.txt")) { SD.remove ("gyro.txt"); }
   if (SD.exists ("accel.txt")) { SD.remove("accel.txt"); }


   //---------------------------------------------------------------------------------------
   // Create the data file and write header information to the file.  This will help us 
   // know what each kind of data is stored in the file.
   
   DateTime now = rtc.now();
   File sensorFile;
   File geigerFile;
   File gyroFile;
   File accelFile;
   
   sensorFile = SD.open("sensor.txt", FILE_WRITE);
   geigerFile = SD.open("geiger.txt", FILE_WRITE);
   gyroFile = SD.open("gyro.txt", FILE_WRITE);
   accelFile = SD.open("accel.txt", FILE_WRITE);

   // Sensors
#if defined (DEBUG)
   sensorFile.println ("DEBUG BUILD");
#endif
   sensorFile.println ("Sensor Data - Team Dan");
   sensorFile.print (  "Mission Date:  "); print_date (now, sensorFile); sensorFile.println ("  Bellevue, WA\n");
   sensorFile.print (  "Time (hh:mm:ss), Pressure (hPa), Temperature (degrees C),");
   sensorFile.println ("Light (lux), Infrared Light (counts), Visible Light (counts)");
   sensorFile.flush();
   DebugSprintf ("sensor.txt opened\n");

   // Geiger
#if defined (DEBUG)
   geigerFile.println("DEBUG BUILD");
#endif
   geigerFile.println ("Geiger Data - Team");
   geigerFile.print ("Mission Date\t"); print_date(now, geigerFile); geigerFile.println("\tBellevue, WA\n");
   geigerFile.print ("Time (hh:mm:ss), CPS, CPM, uSv/hr, str");
   geigerFile.flush();
   DebugSprintf ("geiger.txt opened\n");

   // Gyro
#if defined (DEBUG)
   gyroFile.println("DEBUG BUILD");
#endif
   gyroFile.println("Gyro Data - Team");
   gyroFile.print("Mission Date: \t"); print_date(now, gyroFile); gyroFile.println("\tBellevue, WA\n");
   gyroFile.print("x, y, z");
   gyroFile.flush();
   DebugSprintf("gyro.txt opened\n");

   // Accel
#if defined (DEBUG)
   accelFile.println("DEBUG BUILD");
#endif
   accelFile.println("Gyro Data - Team");
   accelFile.print("Mission Date: \t"); print_date(now, gyroFile); gyroFile.println("\tBellevue, WA\n");
   accelFile.println("1 = m/s^2, 2 = gforce");
   accelFile.print("x1, y1, z1, x2, y2, z2");
   accelFile.flush();
   DebugSprintf("accel.txt opened\n");

   //---------------------------------------------------------------------------------------
   // Wait for start pin to be removed
   
   while (LOW == digitalRead(START_PIN))
   {
      BlinkMorseCode (".");
      DebugSprintf ("Waiting for Start Pin\n");
   }
   DebugSprintf ("Start pin removed\n");


   //---------------------------------------------------------------------------------------
   // Initialize the sensors

   // Pressure sensor gets ID #1
   PressureReading pr1 {};
   Pressure_HSC_I2C pressureSensor { 1 };

   // Light sensor gets ID #2
   LightReading lr1 {};
   Light_TSL2561 tsl2561 {2};
   tsl2561.set_sensitivity (tsl2561::Gain::low, tsl2561::IntegrationTime::T_101ms);

   // Geiger sensor gets ID #3
   GeigerReading gr1 {};
   GeigerCounter geiger { 3 };

   // Gyro gets ID #4
   RotationReading rr1;
   Gyro_L3GD20H gyro { 4 };
   gyro.set_scale(l3gd20::Scale::S_2000dps);

   // Accelerometer gets ID #5
   AccelReading ar1;
   Accel_LSM303 accel { 5 };
   accel.set_scale(lsm303::Scale::S_8g);

   //---------------------------------------------------------------------------------------
   // Loop timing for main program
   
   constexpr uint16_t SENSOR_WRITE_INTERVAL = 100;     // read sensors every 100ms

   uint16_t t1;   // timestamps; used to take action after every xxx milliseconds
   uint16_t t2;

   
   /////////////////////////////////////////////////////////////////////////////////////////
   // Mission runs here

   // Geiger
   Serial.print("Geiger startup time:\t"); Serial.println(geiger.startup_time());
   Serial.print("Geiger response Time:\t"); Serial.println(geiger.response_time());

   // Gyro
   Serial.print("Gyro startup time:\t"); Serial.println(gyro.startup_time());
   Serial.print("Gyro response time:\t"); Serial.println(gyro.response_time());

   // Accel
   Serial.print("Accel startup time:\t"); Serial.println(accel.startup_time());
   Serial.print("Accel response time:\t"); Serial.println(accel.response_time());
   
   DebugSprintf ("Mission Starting\n");

   BlinkMorseCode("-.-.-.-");
   
   bool result;
   int i = 0;

   while (true)
   {
      t1 = millis();

      // Read sensors
      DebugSprintf ("Read sensors\n");
      
      result = pressureSensor.read (pr1);
      result = tsl2561.read (lr1);
      result = geiger.read (gr1);
      result = gyro.read (rr1);
      result = accel.read(ar1);
      now = rtc.now();

      // Write data to sensor file
      print_time(now, sensorFile); sensorFile.print (","); // time
      sensorFile.print (pr1.data.pressure); sensorFile.print (","); sensorFile.print (pr1.data.temp); sensorFile.print (",");
      sensorFile.print (lr1.data.lux); sensorFile.print (","); sensorFile.print (lr1.data.infrared_raw); sensorFile.print (","); sensorFile.println (lr1.data.visible_raw);

      // Write data to geiger file
      print_time(now, geigerFile); geigerFile.print(",");
      geigerFile.print(gr1.data.counts_per_second); geigerFile.print(",");
      geigerFile.print(gr1.data.counts_per_minute); geigerFile.print(",");
      geigerFile.print(gr1.data.radiation); geigerFile.print(",");
      geigerFile.print(gr1.data.str);

      // Write data to gyro file
      print_time(now, gyroFile); gyroFile.print(",");
      gyroFile.print(rr1.data.x); gyroFile.print(",");
      gyroFile.print(rr1.data.y); gyroFile.print(",");
      gyroFile.print(rr1.data.z);

      // Write data to accel file
      print_time(now, accelFile); accelFile.print(",");
      // these values are m/s^2
      accelFile.print(ar1.data.x); accelFile.print(",");
      accelFile.print(ar1.data.y); accelFile.print(",");
      accelFile.print(ar1.data.z); accelFile.print(",");
      // these values are gforce
      result = accel.read_g(ar1);
      accelFile.print(ar1.data.x); accelFile.print(",");
      accelFile.print(ar1.data.y); accelFile.print(",");
      accelFile.print(ar1.data.z);
      
      // Ensure data is written to SD card every N times
      if (++i % 20 == 0)
      {
         DebugSprintf ("Flush data file, i = %d\n", i);
         sensorFile.flush();
         geigerFile.flush();
         gyroFile.flush();
         accelFile.flush();
         BlinkMorseCode("-");
      }    

      t2 = millis();


      // Control mission loop runtime by delaying until next sensor read time
      // While debugging, blink the LED; for actual mission, leave the LED off.
#if defined (DEBUG)
//      DebugSprintf (" t1, t2 = %d, %u\n", t1, t2);
//      DebugSprintf (" (SENSOR_WRITE_INTERVAL -         (t2-t1) ) / 2 = %d\n", (SENSOR_WRITE_INTERVAL -         (t2-t1)) / 2);
//      DebugSprintf (" (SENSOR_WRITE_INTERVAL - uint16_t(t2-t1) ) / 2 = %d\n", (SENSOR_WRITE_INTERVAL - uint16_t(t2-t1)) / 2);
      digitalWrite (LED_PIN, HIGH);
      delay ( (SENSOR_WRITE_INTERVAL - (t2-t1) ) / 2 );
      digitalWrite (LED_PIN, LOW);
      delay ( (SENSOR_WRITE_INTERVAL - (t2-t1) ) / 2 );

#else
//      DebugSprintf (" t1, t2 = %d, %u\n", t1, t2);
//      DebugSprintf (" SENSOR_WRITE_INTERVAL - (t2-t1) = %d\n", SENSOR_WRITE_INTERVAL - (t2-t1) );
      delay (SENSOR_WRITE_INTERVAL - (t2-t1));
#endif

   }

}


/*---------------------------------------------------------------------------------------------
BlinkMorseCode (pin, pattern)

Blinks an LED by pulsing the digital pin to which it is attached by a string that represents
Morse Code.  Example call:

  BlinkMorseCode (LED_PIN, "-.-");  // Pattern: dot (short), dash (long), dot (short)
---------------------------------------------------------------------------------------------*/
void BlinkMorseCode (const char *pattern)
{
   int  pin = LED_PIN;
   char ch;
   int  duration; 

   if (nullptr == pattern)
   {
      DebugSprintf ("BlinkMorseCode called with null pointer\n");
      return;
   }

   while (ch = *pattern++)
   {
      switch (ch)
      {
         case '.':
            duration = 250;
            break;
         case '-':
            duration = 750;
            break;
         default:
            duration = 0; 
      }
      digitalWrite (pin, HIGH);
      delay (duration);
      digitalWrite (pin, LOW);
      delay (200);
   }
}


/*---------------------------------------------------------------------------------------------
print_date (dt, s = Serial)

Prints the date part of a DateTime structure to the specified Stream (Serial by default).
Formats the date into an Excel-friendly format (mm/dd/yyyyy).
---------------------------------------------------------------------------------------------*/
void print_date (DateTime &dt, Stream &s)
{
   constexpr int date_len = 11;  // 12/31/2017 + trailing null
   char str[date_len];

   sprintf (str, "%02d/%02d/%04d", dt.month(), dt.day(), dt.year());
   s.print(str);
}


/*---------------------------------------------------------------------------------------------
print_time (dt, s = Serial)

Prints the time part of a DateTime structure to the specified Stream (Serial by default).
Formats the time into an Excel-friendly format (hh:mm:ss).
---------------------------------------------------------------------------------------------*/
void print_time (DateTime &dt, Stream &s)
{
   constexpr int time_len = 9;  // 22:45:34 + trailing null
   char str[time_len];

   sprintf (str, "%02d:%02d:%02d", dt.hour(), dt.minute(), dt.second());
   s.print(str);
}


void setup() {}

