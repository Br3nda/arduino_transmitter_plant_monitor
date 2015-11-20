// transmitter.pde
//
// Simple example of how to use VirtualWire to transmit messages
// Implements a simplex (one-way) transmitter with an TX-C1 module
//
// See VirtualWire.h for detailed API docs
// Author: Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2008 Mike McCauley
// $Id: transmitter.pde,v 1.3 2009/03/30 00:07:24 mikem Exp $

#include <VirtualWire.h>
#include <dht11.h>

dht11 DHT;

const int led_pin = 11;
const int transmit_pin = 12;
const int dht_sensor_pin = 11;
const String sensor_name = "kitchen";
const bool debug_mode = false;


#define SOIL_PIN_1 7
#define SOIL_PIN_2 5
#define SENSOR_PIN 0
#define TIME_BETWEEN_SOIL_READINGS 1000

void setup()
{
  if (debug_mode) {
    delay(1000);
    Serial.begin(115200);	// Debugging only
    Serial.println("Starting receiver");
  }
  
    // Initialise the IO and ISR
    vw_set_tx_pin(transmit_pin);
    vw_set_ptt_inverted(true); // Required for DR3100
    vw_setup(500);       // Bits per sec
    pinMode(led_pin, OUTPUT);
    
    
    //plant set up
    //set up the plant sensor
    pinMode(SOIL_PIN_1, OUTPUT);
    pinMode(SOIL_PIN_2, OUTPUT);
    pinMode(SENSOR_PIN, INPUT);    
}

void loop()
{
    int chk = DHT.read(dht_sensor_pin);
  switch (chk) {
    case DHTLIB_OK:
      break;
    case DHTLIB_ERROR_CHECKSUM:
      Serial.println("Checksum error,\t");
      break;
    case DHTLIB_ERROR_TIMEOUT:
      Serial.println("Time out error,\t");
      break;
    default:
      Serial.println("Unknown error,\t");
      break;
  }
  
  float temp     = (float) DHT.temperature;
  float humidity = (float) DHT.humidity;
  
  String message;
  
  message = "R>" + sensor_name + ":temp=" + (String) temp; // + ";humidity=" + (String) humidity;
  sendMessage(message);
  
  message = "R>" + sensor_name + ":humidity=" + (String) humidity;
  sendMessage(message);
  
  float moisture = readPlantMoisture();
  message = "R>" + sensor_name + ":moisture=" + (String) moisture;
  sendMessage(message);
  
  //delay(TIME_BETWEEN_SOIL_READINGS);
  delay(10 * 1000); // ensure we don't read too often
}
void sendMessage(String message) { 
  if (debug_mode) {
    Serial.println(message);
  }
  int buf_len = message.length() +1;
  
  char buf[buf_len];
  message.toCharArray(buf, buf_len);
  
  digitalWrite(led_pin, HIGH); // Flash a light to show transmitting
  vw_send((uint8_t *)buf, buf_len);
  vw_wait_tx(); // Wait until the whole message is gone
  digitalWrite(led_pin, LOW);
  delay(1000);
}


void setSensorPolarity(boolean flip) {
  if (flip) {
    digitalWrite(SOIL_PIN_1, HIGH);
    digitalWrite(SOIL_PIN_2, LOW);
  }
  else {
    digitalWrite(SOIL_PIN_1, LOW);
    digitalWrite(SOIL_PIN_2, HIGH);
  }
}

float readPlantMoisture() {
  /**
  Read moisture (voltage change) across 2 pins
  Then read in reverse.
  Alternating polatity makes the metal in the pot last longer.
  */
  
  //first read
  setSensorPolarity(true);
  delay(TIME_BETWEEN_SOIL_READINGS);
  int val1 = analogRead(SENSOR_PIN);
  delay(TIME_BETWEEN_SOIL_READINGS);


  //second read, opposite polarity
  setSensorPolarity(false);
  delay(TIME_BETWEEN_SOIL_READINGS);
  // invert the reading
  int val2 = 1023 - analogRead(SENSOR_PIN);

  float avg = (val1 + val2) / 2;

  return avg;
}




