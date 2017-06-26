#ifndef CONFIG_H
#define CONFIG_H

#include <arduino.h>

//pin set for Arduino Leonardo
const int SOIL1_IN = 10;
const int SOIL2_IN = 16;
const int SOIL3_IN = 14;
const int SOIL4_IN = 15;
const int WATER_IN = 2;
const int AIR_IN = 7;
const int POT_IN = A0;
const int SWITCH1_IN = 3;
const int SWITCH2_IN = 4;
const int RELAY1_OUT = 8;
const int RELAY2_OUT = 9;
const int BUZZER_OUT = 5;
const int AIR_LED_OUT = 6;

//max watering time in one turn on cycle
const int MAX_WATERING_TIME_SEC = 30;

#endif
