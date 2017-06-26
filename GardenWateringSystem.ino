#include <avr/wdt.h>
#include "config.h"
#include "custom_interface.h"

#define TEST 1
//auxiliary variable for serial printing
unsigned int i = 0;

void setup() 
{ 
  if(TEST) Serial.begin(9600);
}

void loop()
{
  ++i;

  //watch dog enable
  wdt_enable(WDTO_1S);
  interface::readAndControl();  //set of functions grouped in order do read sensors and control pumps
  wdt_reset();

  if(TEST && i%10000 == 0)   //i=10000 is about 4 seconds
    interface::printInfo();
}
