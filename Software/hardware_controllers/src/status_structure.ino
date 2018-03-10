
void print_value_float(char * name, float  value){
  Serial.print("\"");
  Serial.print(name);
  Serial.print("\":");
  Serial.print(isnan(value)? 0 : value);
}

void print_value_int(char * name, int  value){
  Serial.print("\"");
  Serial.print(name);
  Serial.print("\":");
  Serial.print(isnan(value)? 0 : value);
}


void print_value_bool(char * name, bool  value){
  Serial.print("\"");
  Serial.print(name);
  Serial.print("\":");
  Serial.print(value);
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
  print_value_float("min_tmp", cs->min_tmp);
  Serial.print(",");
  print_value_bool("light", cs->light);
  Serial.print(",");
  print_value_int("visible_lux", cs->visible_lux);
  Serial.print("}}");
  Serial.println();
}

void status_read_environment(struct CurrentStatus * cs){
  float hum = dht_humidity();
  float temp = dht_temperature();
  if(hum == 0 || temp == 0){
    cs-> missed_temp_reads++;
  }else{
    cs->humidity = hum;
    cs->temperature = temp;
    cs-> missed_temp_reads = 0;
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
  relay_on(PIN_PELTIER);
  fan_set_speed(PIN_FAN_INSIDE, 0);
  fan_set_speed(PIN_FAN_OUTSIDE, 0);
  cs->peltier_cool_status = true;
}

void status_stop_cool(struct CurrentStatus * cs){
  relay_off(PIN_PELTIER);
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
  if(cs->missed_temp_reads > 10){
    Serial.println("{\"ERROR\": \" Temperature not read \"");
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
    int tmp_val;
    if(messageFromPC == "PRINT"){
      print_sensor_error(cs);
      status_print(cs);
    }else if(messageFromPC == "max_tmp"){
      strtokIndx = strtok(NULL, ",");
      cs->max_tmp = atof(strtokIndx);
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
    }
    cs->new_data = false;
  }
}

void status_control_temperature(struct CurrentStatus * cs){
  cs->next_peltier_cool_status = cs->temperature > cs->max_tmp;
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
  cs->visible_lux = TSL2561.readVisibleLux();
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
