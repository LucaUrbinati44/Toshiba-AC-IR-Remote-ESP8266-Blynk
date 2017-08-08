/*************************************************************
  This code is used to control a Toshiba Air Conditioner remotely from your Blynk App.
  Written by Luca Urbinati: https://github.com/LucaUrbinati44

  WARNING :
  For this example you'll need these libraries:
    - Blynk library:
      https://github.com/blynkkk/blynk-library/releases/latest
    - Adafruit DHT sensor libraries:
      https://github.com/adafruit/Adafruit_Sensor
      https://github.com/adafruit/DHT-sensor-library
    - Toshiba AC library, from mattiarossi/HVAC-IR-Control on Github.
      https://github.com/mattiarossi/HVAC-IR-Control/tree/master/HVAC_ESP8266
  
  For this example you'll need these components:
    - Wemos D1 board or equivalent, that means based on ESP8266.
    - DHT11 Temperature and Humidity sensor, to have a feedback from the environment.
    - IR LED to a PWM pin, optionally with a transistor npn to increase its range.

  For the connections of DHT11 and IR LED surf on the Internet.
  
  Blynk App project setup:
    see the picture Toshiba_ac_blynk.jpg in the folder of this sketch on Github
    
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

/*****************************************************************/
// TOSHIBA LIBRARY from Mattia Rossi: https://github.com/mattiarossi/HVAC-IR-Control
/****************************************************************************/
int halfPeriodicTime;
int IRpin;
int khz;

typedef enum HvacMode {
  HVAC_HOT,
  HVAC_COLD,
  HVAC_DRY,
  HVAC_FAN, // used for Panasonic only
  HVAC_AUTO
} HvacMode_t; // HVAC  MODE

typedef enum HvacFanMode {
  FAN_SPEED_1,
  FAN_SPEED_2,
  FAN_SPEED_3,
  FAN_SPEED_4,
  FAN_SPEED_5,
  FAN_SPEED_AUTO,
  FAN_SPEED_SILENT
} HvacFanMode_;  // HVAC  FAN MODE

typedef enum HvacVanneMode {
  VANNE_AUTO,
  VANNE_H1,
  VANNE_H2,
  VANNE_H3,
  VANNE_H4,
  VANNE_H5,
  VANNE_AUTO_MOVE
} HvacVanneMode_;  // HVAC  VANNE MODE

typedef enum HvacWideVanneMode {
  WIDE_LEFT_END,
  WIDE_LEFT,
  WIDE_MIDDLE,
  WIDE_RIGHT,
  WIDE_RIGHT_END
} HvacWideVanneMode_t;  // HVAC  WIDE VANNE MODE

typedef enum HvacAreaMode {
  AREA_SWING,
  AREA_LEFT,
  AREA_AUTO,
  AREA_RIGHT
} HvacAreaMode_t;  // HVAC  WIDE VANNE MODE

typedef enum HvacProfileMode {
  NORMAL,
  QUIET,
  BOOST
} HvacProfileMode_t;  // HVAC PANASONIC OPTION MODE


// HVAC TOSHIBA_
#define HVAC_TOSHIBA_HDR_MARK    4400
#define HVAC_TOSHIBA_HDR_SPACE   4300
#define HVAC_TOSHIBA_BIT_MARK    543
#define HVAC_TOSHIBA_ONE_SPACE   1623
#define HVAC_MISTUBISHI_ZERO_SPACE  472
#define HVAC_TOSHIBA_RPT_MARK    440
#define HVAC_TOSHIBA_RPT_SPACE   7048 // Above original iremote limit


/****************************************************************************
/* Send IR command to Toshiba HVAC - sendHvacToshiba
/***************************************************************************/
void sendHvacToshiba(
  HvacMode                HVAC_Mode,           // Example HVAC_HOT  
  int                     HVAC_Temp,           // Example 21  (°c)
  HvacFanMode             HVAC_FanMode,        // Example FAN_SPEED_AUTO  
  int                     OFF                  // Example false
)
{
 
#define HVAC_TOSHIBA_DATALEN 9
#define  HVAC_TOSHIBA_DEBUG;  // Un comment to access DEBUG information through Serial Interface

  byte mask = 1; //our bitmask
  //﻿F20D03FC0150000051
  byte data[HVAC_TOSHIBA_DATALEN] = { 0xF2, 0x0D, 0x03, 0xFC, 0x01, 0x00, 0x00, 0x00, 0x00 };
  // data array is a valid trame, only byte to be chnaged will be updated.

  byte i;

#ifdef HVAC_TOSHIBA_DEBUG
  Serial.println("Packet to send: ");
  for (i = 0; i < HVAC_TOSHIBA_DATALEN; i++) {
    Serial.print("_");
    Serial.print(data[i], HEX);
  }
  Serial.println(".");
#endif

  data[6] = 0x00;
  // Byte 7 - Mode
  switch (HVAC_Mode)
  {
    case HVAC_HOT:   data[6] = (byte) B00000011; break;
    case HVAC_COLD:  data[6] = (byte) B00000001; break;
    case HVAC_DRY:   data[6] = (byte) B00000010; break;
    case HVAC_AUTO:  data[6] = (byte) B00000000; break;
    default: break;
  }


  // Byte 7 - On / Off
  if (OFF) {
    data[6] = (byte) 0x07; // Turn OFF HVAC
  } else {
     // Turn ON HVAC (default)
  }

  // Byte 6 - Temperature
  // Check Min Max For Hot Mode
  byte Temp;
  if (HVAC_Temp > 30) { Temp = 30;}
  else if (HVAC_Temp < 17) { Temp = 17; } 
  else { Temp = HVAC_Temp; };
  data[5] = (byte) Temp - 17<<4;

  // Byte 10 - FAN / VANNE
  switch (HVAC_FanMode)
  {
    case FAN_SPEED_1:       data[6] = data[6] | (byte) B01000000; break;
    case FAN_SPEED_2:       data[6] = data[6] | (byte) B01100000; break;
    case FAN_SPEED_3:       data[6] = data[6] | (byte) B10000000; break;
    case FAN_SPEED_4:       data[6] = data[6] | (byte) B10100000; break;
    case FAN_SPEED_5:       data[6] = data[6] | (byte) B11000000; break; 
    case FAN_SPEED_AUTO:    data[6] = data[6] | (byte) B00000000; break;
    case FAN_SPEED_SILENT:  data[6] = data[6] | (byte) B00000000; break;//No FAN speed SILENT for TOSHIBA so it is consider as Speed AUTO
    default: break;
  }

  // Byte 9 - CRC
  data[8] = 0;
  for (i = 0; i < HVAC_TOSHIBA_DATALEN - 1; i++) {
    data[HVAC_TOSHIBA_DATALEN-1] = (byte) data[i] ^ data[HVAC_TOSHIBA_DATALEN -1];  // CRC is a simple bits addition
  }

#ifdef HVAC_TOSHIBA_DEBUG
  Serial.println("Packet to send: ");
  for (i = 0; i < HVAC_TOSHIBA_DATALEN; i++) {
    Serial.print("_"); Serial.print(data[i], HEX);
  }
  Serial.println(".");
  for (i = 0; i < HVAC_TOSHIBA_DATALEN ; i++) {
    Serial.print(data[i], BIN); Serial.print(" ");
  }
  Serial.println(".");
#endif

  enableIROut(38);  // 38khz
  space(0);
  for (int j = 0; j < 2; j++) {  // For Mitsubishi IR protocol we have to send two time the packet data
    // Header for the Packet
    mark(HVAC_TOSHIBA_HDR_MARK);
    space(HVAC_TOSHIBA_HDR_SPACE);
    for (i = 0; i < HVAC_TOSHIBA_DATALEN; i++) {
      // Send all Bits from Byte Data in Reverse Order
      for (mask = 10000000; mask > 0; mask >>= 1) { //iterate through bit mask
        if (data[i] & mask) { // Bit ONE
          mark(HVAC_TOSHIBA_BIT_MARK);
          space(HVAC_TOSHIBA_ONE_SPACE);
        }
        else { // Bit ZERO
          mark(HVAC_TOSHIBA_BIT_MARK);
          space(HVAC_MISTUBISHI_ZERO_SPACE);
        }
        //Next bits
      }
    }
    // End of Packet and retransmission of the Packet
    mark(HVAC_TOSHIBA_RPT_MARK);
    space(HVAC_TOSHIBA_RPT_SPACE);
    space(0); // Just to be sure
  }
}

/****************************************************************************
/* enableIROut : Set global Variable for Frequency IR Emission
/***************************************************************************/ 
void enableIROut(int khz) {
  // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  halfPeriodicTime = 500/khz; // T = 1/f but we need T/2 in microsecond and f is in kHz
}

/****************************************************************************
/* mark ( int time) 
/***************************************************************************/ 
void mark(int time) {
  // Sends an IR mark for the specified number of microseconds.
  // The mark output is modulated at the PWM frequency.
  long beginning = micros();
  while(micros() - beginning < time){
    digitalWrite(IRpin, HIGH);
    delayMicroseconds(halfPeriodicTime);
    digitalWrite(IRpin, LOW);
    delayMicroseconds(halfPeriodicTime); //38 kHz -> T = 26.31 microsec (periodic time), half of it is 13
  }
}

/****************************************************************************
/* space ( int time) 
/***************************************************************************/ 
/* Leave pin off for time (given in microseconds) */
void space(int time) {
  // Sends an IR space for the specified number of microseconds.
  // A space is no output, so the PWM output is disabled.
  digitalWrite(IRpin, LOW);
  if (time > 0) delayMicroseconds(time);
}

/****************************************************************************
/* sendRaw (unsigned int buf[], int len, int hz)
/***************************************************************************/ 
void sendRaw (unsigned int buf[], int len, int hz)
{
  enableIROut(hz);
  for (int i = 0; i < len; i++) {
    if (i & 1) {
      space(buf[i]);
    } 
    else {
      mark(buf[i]);
    }
  }
  space(0); // Just to be sure
}


/****************************************************************************/
// HERE THE CODE STARTS
/****************************************************************************/

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "c9053214a5f54a1089eb1b2207a38736";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Casa";
char pass[] = "lucaurbinatilucaurbinati";

// Variables
// These declarations are equivalent.
// If you use the number, it is the GPIO pin of the ESP8266
//#define DHTPIN 5 //(GPI05 --> D1)     
// If you use the digital pin of the Wemos D1, 
// you need to know to which GPIO pin of the ESP8266 it is connected.
#define DHTPIN D1 //(GPI05 --> D1)     

bool globalState; // if 1, the AC is on; if 0, otherwise.
int targetTemp; // the temperature you set from the Blynk App.
int tempAmb; // the temperature read by the sensor.
int humAmb; // the humidity read by the sensor.
int startTemp; // the temperature read by the sensor when the AC is whitched on.
int goalTemp; // the temperature difference you want to achieve.
unsigned long startTime; // the timer when startTemp is read.
unsigned long waiting; // the time after which Wemos D1 cheaks if the goalTemp has been reached.
int sent; // varibale that store the number of times the notifications are sent.
int sentError; // varibale that store the number of times the error notifications are sent.

// Built in LED for debugging.
//#define LEDPIN LED_BUILTIN //(GPIO2 --> D4)     
//#define LEDPIN D4 //(GPIO2 --> D4) it's the same

// Uncomment whatever type you're using!
#define DHTTYPE DHT11     // DHT 11
//#define DHTTYPE DHT22   // DHT 22, AM2302, AM2321
//#define DHTTYPE DHT21   // DHT 21, AM2301

DHT dht(DHTPIN, DHTTYPE);

BlynkTimer timer;


/****************************************************************************/
// Send temperature and humidity to Blynk App.
void sendSensor()
{
  humAmb = dht.readHumidity();
  tempAmb = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(humAmb) || isnan(tempAmb) || humAmb > 100) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(humAmb);
  Serial.print(" - Temperature: ");
  Serial.println(tempAmb);
  Serial.println();
  
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V3, humAmb);
  Blynk.virtualWrite(V2, tempAmb);

  if(globalState == 1 && (millis() - startTime) > waiting) {
    
    if((startTemp - tempAmb) >= goalTemp && sent < 3){
      Serial.println("The AC has cooled the room");
      // Send notifications.
      if (sent == 2) {
        Blynk.email("luca.urbinati.44@gmail.com", "Target temp reached", "The target temperature has been reached. The AC has cooled the room.");
        delay(20);
        Blynk.notify("Target temp reached: the AC has cooled the room");
        Serial.println("Successfull notifications sent");
        sent++;
        return;
      }
      Blynk.email("luca.urbinati.44@gmail.com", "Target temp seems reached", "The target temperature seems to be reached. To be sure, wait for the confirmation message.");
      delay(20);
      Blynk.notify("Target temp seems reached: wait the confirmation message.");
      Serial.println("Notifications sent");
      // Restart the timer.
      startTime = millis();
      sent++;
      
    } else if(sentError < 3) {
      // Send error notifications.
      Blynk.email("luca.urbinati.44@gmail.com", "Error", "The AC can't cool down the room. Maybe you left a door or a window open. If not, try to restart the AC.");
      delay(20);
      Blynk.notify("Error: the AC can't cool down the room. Maybe you left a door or a window open. If not, try to restart the AC.");
      Serial.println("Error notifications sent");
      // Restart the timer.
      startTime = millis();
      sentError++;
    }
  }
}
/****************************************************************************/

// This function will be called every time Button Widget
// in Blynk app writes values to the Virtual Pin 0 (ON)
BLYNK_WRITE(V0)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if (pinValue == 1 && globalState == 0) {    
    // You can also use:
    // String i = param.asStr();
    // double d = param.asDouble();

    // Save the temperature before to turn on the AC.
    startTemp = tempAmb;
    // Start the timer.
    startTime = millis();
    // Reset the sent variables.
    sent = 0;
    sentError = 0;

    // Turn the AC on.
    sendHvacToshiba(HVAC_COLD, targetTemp, FAN_SPEED_1, false);
    //digitalWrite(LEDPIN, !HIGH);
    Serial.println("The AC is ON");

    globalState = 1;
    
  } else if (pinValue == 0 && globalState == 1) { 
    // Turn the AC off. You need to use FAN_SPEED_AUTO and true, otherwise it goes in sleep mode.
    sendHvacToshiba(HVAC_COLD, targetTemp, FAN_SPEED_AUTO, true);
    //digitalWrite(LEDPIN, !LOW);
    Serial.println("The AC is OFF");
    globalState = 0;
  }
}
/****************************************************************************/

// Wemos D1 enters here every time the slider changes. 
BLYNK_WRITE(V1)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  targetTemp = pinValue;
  // At least half of the difference should be reached, considering the inaccuracies of the DHT11 sensor.
  goalTemp = (startTemp-targetTemp)/2;  // example: (30-20)/2 = 5.

  if (globalState == 1) {
    // Change temperature.  
    sendHvacToshiba(HVAC_COLD, targetTemp, FAN_SPEED_1, false);
    // Save the temperature before to turn on the AC.
    startTemp = tempAmb;
    // Start the timer.
    startTime = millis();
    // Reset the sent variables.
    sent = 0;
    sentError = 0;
  }
}
/****************************************************************************/

// Synchronize Blynk app values with those of the Wemos D1.
void synchValues() {
  globalState = 0;
  Blynk.virtualWrite(V0, globalState);
  startTemp = 50;
  targetTemp = 25;
  goalTemp = (28-25)/2;
  waiting = 10*60*1000; // 10 min
  Blynk.virtualWrite(V1, targetTemp);
  tempAmb = 0;
  Blynk.virtualWrite(V2, tempAmb);
  humAmb = 0;
  Blynk.virtualWrite(V3, humAmb);
}
/****************************************************************************/

// Keep in mind that when sleeping all data contained in variables are lost 
// and each time the cpu wakes up from setup(). So you'll loose all the previous states.
void setup()
{
  // Debug console
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8442);

  // Synchronize Blynk app values with those of the Wemos D1.
  synchValues();
  
  // LED
  //pinMode(LEDPIN, OUTPUT);
  //delay(10);
  //digitalWrite(LEDPIN, !LOW);  
  
  // IR LED
  // These data are taken from the examples of the Toshiba library.
  IRpin=D2;
  khz=38;
  halfPeriodicTime = 500/khz;
  pinMode(IRpin, OUTPUT);

  dht.begin();

  // In the app, Widget's reading frequency should be set to PUSH. This means
  // that you define how often to send data to Blynk App.
  // Setup a function to be called every 15 seconds.
  timer.setInterval(15000L, sendSensor);
}
/****************************************************************************/

void loop()
{
  Blynk.run();
  timer.run();
}
/****************************************************************************/



