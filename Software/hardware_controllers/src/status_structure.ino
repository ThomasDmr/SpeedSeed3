void status_clear_in_buffer(){
  while(Serial.available())
    Serial.read();
}


void print_value_float(char * name, float  value){
  Serial.print("\"");
  Serial.print(name);
  Serial.print("\":");
  Serial.print(isnan(value)? 0 : value);
  Serial.flush();
}

void print_value_int(char * name, int  value){
  Serial.print("\"");
  Serial.print(name);
  Serial.print("\":");
  Serial.print(isnan(value)? 0 : value);
  Serial.flush();
}


void print_value_bool(char * name, bool  value){
  Serial.print("\"");
  Serial.print(name);
  Serial.print("\":");
  Serial.print(value);
  Serial.flush();
}

void status_print(struct CurrentStatus * cs){

  Serial.print("{\"status\":{");
  print_value_float("humidity", cs->humidity);
  Serial.print(",");
  print_value_float("temperature", cs->temperature);
  Serial.print(",");
  print_value_bool("peltier_cool_status", cs->peltier_cool_status);
  Serial.print(",");
  print_value_bool("humidity_fan_status", cs->humidity_fan_status);
  Serial.print(",");
  print_value_float("max_humidity", cs->max_humidity);
  Serial.print(",");
  print_value_float("max_tmp", cs->max_tmp);
  Serial.print(",");
  print_value_float("water_volume", cs->water_volume);
  Serial.print(",");
  print_value_float("min_tmp", cs->min_tmp);
  Serial.print(",");
  print_value_bool("light", cs->light);
  Serial.print(",");
  print_value_int("visible_lux", cs->visible_lux);
  Serial.print(",");
  print_value_int("soil_moisture", cs->soil_moisture);
  Serial.print(",");
  print_value_int("missed_temp_reads", cs->missed_temp_reads);
  Serial.print(",");
  print_value_bool("bme280", cs->bme280);
  Serial.print(",");
  print_value_bool("bmp280", cs->bmp280);
  Serial.print("}}");

  Serial.println();
  Serial.flush();
}

void status_read_environment(struct CurrentStatus * cs){
  float hum, temp;
  int moisture; 

  if(cs->bme280){
    hum  = bme280_humidity();
    temp = bme280_temperature();
  }else if(cs->bmp280){
    hum  = bmp280_humidity();
    temp = bmp280_temperature();
  }else{
    hum  = dht_humidity();
    temp = dht_temperature();
  }

  moisture = constrain(analogRead(PIN_MOISTURE_SENSOR), MOISTURE_WATER, MOISTURE_AIR);
  moisture = map(moisture, MOISTURE_WATER, MOISTURE_AIR, 100, 0);

  if(!isnan(hum)){
    cs->humidity = hum;
    cs-> missed_temp_reads = 0;
  }

  if(!isnan(temp)){
    cs->temperature = temp;
    cs-> missed_temp_reads = 0;
  }

  if( isnan(temp)){
    cs-> missed_temp_reads = cs-> missed_temp_reads + 1;
  }

  if(!isnan(moisture)){
    static uint32_t temps = millis();
    if(millis() - temps > 1000)
    {
      DEBUG_PRINTLN("Moisture: " + String(moisture));
      DEBUG_PRINTLN("Temp: " + String(temp));
      DEBUG_PRINTLN("Humidity: " + String(hum));
      temps = millis();
    }

    cs->soil_moisture = moisture;
  }

}

void status_start_light(struct CurrentStatus * cs){
  relay_on(PIN_LIGHT);
  cs->light = true;
}

void status_stop_light(struct CurrentStatus * cs){
  relay_off(PIN_LIGHT);
  cs->light = false;
}



void status_start_cool(struct CurrentStatus * cs){
  relay_on(PIN_PELTIER_1);
  relay_on(PIN_PELTIER_2);
  relay_on(PIN_PELTIER_3);
  fan_set_speed(PIN_FAN_INSIDE, 0);
  fan_set_speed(PIN_FAN_OUTSIDE, 0);
  cs->peltier_cool_status = true;
}

void status_stop_cool(struct CurrentStatus * cs){
  relay_off(PIN_PELTIER_1);
  relay_off(PIN_PELTIER_2);
  relay_off(PIN_PELTIER_3);
  fan_set_speed(PIN_FAN_INSIDE, 255);
  fan_set_speed(PIN_FAN_OUTSIDE, 255);
  cs->peltier_cool_status = false;
}

void recvWithEndMarker(struct CurrentStatus * cs) {
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  while (Serial.available() > 0 && cs->new_data == false) {
     rc = Serial.read();
     if (rc != endMarker) {
      cs->receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
     }else {
      cs->receivedChars[ndx] = '\0'; // terminate the string
      ndx = 0;
      cs->new_data = true;
    }
  }
}

void print_sensor_error(struct CurrentStatus * cs){
  if(cs->missed_temp_reads > 20){
    Serial.print("{\"ERROR\": \" Temperature not read \"}");
    Serial.println();
      Serial.flush();
    cs->missed_temp_reads = 0;
  }
  if(cs->missed_lux_reads > 20){
    Serial.print("{\"ERROR\": \" Lux not read \"}");
    Serial.println();
      Serial.flush();
    cs->missed_temp_reads = 0;
  }
}

void parse_new_data(struct CurrentStatus * cs) {
  if (cs->new_data == true) {
    String messageFromPC;
    char * strtokIndx; // this is used by strtok() as an index
    strtokIndx = strtok(cs->receivedChars,"=");
    messageFromPC = String(strtokIndx); // copy it to messageFromPC
    Serial.print("{\"CMD\":\"");
    Serial.print(messageFromPC);
    Serial.print("\"}");
    Serial.println();
    Serial.flush();
    int tmp_val;
    if(messageFromPC == "PRINT"){
      print_sensor_error(cs);
      status_print(cs);
    }else if(messageFromPC == "max_tmp"){
      strtokIndx = strtok(NULL, ",");
      cs->max_tmp = atof(strtokIndx);
      cs->min_tmp = cs->max_tmp -1;
      cs->started_up = false;
      status_print(cs);
    }else if(messageFromPC == "min_tmp"){
      strtokIndx = strtok(NULL, ",");
      //TODO: Add the code to the python module to control thi
      cs->min_tmp = atof(strtokIndx);
      cs->started_up = false;
      status_print(cs);
    }else if (messageFromPC == "max_humidity") {
      strtokIndx = strtok(NULL, ",");
      cs->max_humidity = atof(strtokIndx);
      cs->started_up = false;
      status_print(cs);
    }else if (messageFromPC == "light") {
      strtokIndx = strtok(NULL, ",");
      tmp_val = atoi(strtokIndx);
      cs->next_light = tmp_val > 0;
      cs->started_up = false;
      status_print(cs);
    }
    else{
      Serial.print("{\"ERROR\": \"Unknown command '");
      Serial.print(messageFromPC);
      Serial.print("'\"}");
      Serial.println();
      Serial.flush();
    }
    status_clear_in_buffer();
    cs->new_data = false;
  }
}

void status_control_temperature(struct CurrentStatus * cs){
  if(cs->peltier_cool_status && cs->temperature > cs->min_tmp){
    cs->next_peltier_cool_status = true;
  }else{
    cs->next_peltier_cool_status = cs->temperature > cs->max_tmp;
  }

  if( !cs->started_up || cs->peltier_cool_status != cs->next_peltier_cool_status){
    if(cs->next_peltier_cool_status){
      status_start_cool(cs);
    }else{
      status_stop_cool(cs);
    }
  }
}

void status_control_light(struct CurrentStatus * cs){
  if(!cs->started_up || cs->next_light != cs->light){
    if(cs->next_light){
      status_start_light(cs);
    }else{
      status_stop_light(cs);
    }
  }

  /*int lux =  TSL2561.readVisibleLux();
    if(!isnan(lux) ){
      cs->visible_lux = lux;
    }else{
      cs->missed_lux_reads += 1;
    }
*/
}

void status_control_pump(struct CurrentStatus * cs)
{
  const int           min_moisture = 10; // moisture level under which the pump starts
  const unsigned long pump_run_time = 5000; // time during which the pump runs
  const unsigned int  min_pump_interval = 20000; // time the system waits between two pumping sessions

  static unsigned long pump_start_time = 0;
  static unsigned long pump_last_run_time = 0;

  if(pump_start_time != 0 && millis() - pump_start_time > pump_run_time)
  {
    DEBUG_PRINTLN("STOP PUMP");
    relay_off(PIN_PUMP);
    pump_start_time = 0;
    cs->water_volume += 0.028 * (float)pump_run_time / 1000;
    pump_last_run_time = millis();
  }
  else if(pump_start_time != 0)
  {
    //pump is still running
    static uint32_t temps3 = millis();
    if(millis() - temps3 > 500)
    {
      DEBUG_PRINTLN("Pump Running");
      temps3 = millis();
    }
  }
  else if(cs->soil_moisture < min_moisture && millis() - pump_last_run_time > min_pump_interval)
  {
    DEBUG_PRINTLN("START PUMP " + String(cs->soil_moisture));
    relay_on(PIN_PUMP);
    pump_start_time = millis();
  }
  else
  {
    static uint32_t temps2 = millis();
    if(millis() - temps2 > 2000)
    {
      DEBUG_PRINTLN("Pump Off");
      temps2 = millis();
    }

    relay_off(PIN_PUMP);
    pump_start_time = 0;
  }
}

void status_control_humidity(struct CurrentStatus * cs){
  cs->next_humidity_fan_status = cs->humidity > cs->max_humidity;
  if(!cs->started_up || cs->humidity_fan_status != cs->next_humidity_fan_status){
    if(cs->next_humidity_fan_status){
      relay_on(PIN_HUMIDITY_FAN);
      cs->humidity_fan_status = true;
    }else{
      relay_off(PIN_HUMIDITY_FAN);
      cs->humidity_fan_status = false;
    }
  }
}
