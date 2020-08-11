#include <DHT.h>
#include <Wire.h>
//#include <Digital_Light_TSL2561.h>
#include <math.h>

// Comment next line if you want to disable debug printing
//#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINTLN(x)
#endif

//Pin 14 is A0
//Pin 15 is A1
//Pin 16 is A2
//Pin 17 is A3
//Pin 18 is A8

#include "datastructures.h"

#define PIN_PELTIER_1      4 //Relay Shield (4)
#define PIN_PELTIER_2      5 //Relay Shield (3)
#define PIN_HUMIDITY_FAN 6 //Relay Shield (2)
#define PIN_PELTIER_3      7 //Relay Shield (1)
#define PIN_LIGHT        9
#define PIN_PUMP    8

#define PIN_TMP_SENSOR   A2 //A2
#define PIN_MOISTURE_SENSOR A0

#define MOISTURE_AIR 580
#define MOISTURE_WATER 320

//#define PIN_FAN_PULSE_INSIDE   A0
//#define PIN_FAN_PULSE_OUTSIDE  A1

#define PIN_FAN_INSIDE   A3
#define PIN_FAN_OUTSIDE  A4

#define MAX_TEMPERATURE 30
#define MAX_HUMIDITY 70.0

#define LOOP_WAIT 10000

struct CurrentStatus cs;
//SFE_TSL2561 light;


void setup() {
  Serial.begin(9600);

  bool bmp280 = false;
  bool bme280 = false;

  //Serial.print("Starting setup\n");

  //if(!bmp280){
  //  bmp280 = bmp280_setup();
  //}
  //Serial.print("BMP280");
  if(!bme280){
    bme280 = bme280_setup();
  }
  //bool bme280 = false;

  relay_setup(PIN_PELTIER_1);
  relay_setup(PIN_PELTIER_2);
  relay_setup(PIN_PELTIER_3);
  relay_setup(PIN_HUMIDITY_FAN);
  relay_setup(PIN_LIGHT);
  relay_setup(PIN_PUMP);
//  TSL2561.init();
if(!bme280 && !bmp280){
  dht_setup();
}

  cs.humidity = 0;
  cs.temperature = 0;
  cs.fan1_hz = 0;
  cs.fan2_hz = 0;
  cs.max_tmp = MAX_TEMPERATURE;
  cs.min_tmp = 0;
  cs.max_humidity = MAX_HUMIDITY;
  cs.peltier_cool_status = false;
  cs.next_peltier_cool_status = false;
  cs.humidity_fan_status = false;
  cs.next_humidity_fan_status = false;
  cs.new_data  =  false;
  cs.light = false;
  cs.next_light = false;
  cs.started_up = false;
  cs.missed_temp_reads = 0;
  cs.bme280 = bme280;
  cs.bmp280 = bmp280;
  cs.water_volume = 0;
  cs.soil_moisture = 0;
  status_clear_in_buffer();
  //Serial.print("Done setup");


}

void loop() {
  delay(100);
  //Serial.print("Reading env");
  status_read_environment(&cs);
 //Serial.print("Reading temp");
  status_control_temperature(&cs);
  //status_control_humidity(&cs);
 //Serial.print("Reading light");
  status_control_light(&cs);

  status_control_pump(&cs);
  
  cs.started_up = true;
  //Serial.print(".");
  if (Serial.available() > 0) {
    //Serial.print("AVailable!\n");
    //Serial.flush();
    recvWithEndMarker(&cs);
  }
  if(cs.new_data){
    parse_new_data(&cs);
   }
}
