#include "config.h"
#include "sensors.h"
#include "pumps.h"
#include "custom_interface.h"

AirSensor air_sensor(AIR_IN, AIR_LED_OUT, digitalPinToInterrupt(AIR_IN), interface::dht11Wrapper);
//SoilSensorSegment soil_sensor_segment1(SOIL1_IN, SOIL2_IN, 1);
//SoilSensorSegment soil_sensor_segment2(SOIL3_IN, SOIL4_IN, 2);
Switch switch1(SWITCH1_IN);
Switch switch2(SWITCH2_IN);
WaterSensor water_sensor(WATER_IN, BUZZER_OUT, interface::waterSensorWrapper);

//PumpSS pump1(RELAY1_OUT, POT_IN, MAX_WATERING_TIME_SEC, 1, &switch1, &water_sensor, &air_sensor, &soil_sensor_segment1);
//PumpSS pump2(RELAY2_OUT, POT_IN, MAX_WATERING_TIME_SEC, 2, &switch2, &water_sensor, &air_sensor, &soil_sensor_segment2);
PumpWT pump1(RELAY1_OUT, POT_IN, MAX_WATERING_TIME_SEC, 1, &switch1, &water_sensor, &air_sensor);
PumpWT pump2(RELAY2_OUT, POT_IN, MAX_WATERING_TIME_SEC, 2, &switch2, &water_sensor, &air_sensor);

namespace interface
{
  void dht11Wrapper() 
  {
    air_sensor.isrCallback();
  }

  void waterSensorWrapper()
  {
    water_sensor.readSensor();
  }

  //set of functions for reading sensors and controlling pumps
  void readAndControl()
  {
    air_sensor.readSensor();
    //soil_sensor_segment1.readSensor();
    //soil_sensor_segment2.readSensor();
    switch1.readSensor();
    switch2.readSensor();
    pump1.controlPump();
    pump2.controlPump();
  }

  //wrapper for printing system informarion
  void printInfo()
  {
      air_sensor.printInfo();
      //soil_sensor_segment1.printInfo();
      //soil_sensor_segment2.printInfo();
      pump1.printInfo();
      pump2.printInfo();
      water_sensor.printInfo();
      Serial.print("TIMESTAMP (s): ");
      Serial.println(millis()/1e3);
      Serial.println();
  }
}
