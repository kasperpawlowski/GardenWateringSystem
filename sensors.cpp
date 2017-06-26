#include "sensors.h"

BaseSensor::BaseSensor(const int read_every_sec, const int ready_after_sec) :
  shouldBeWatered_(false), timeLastSec_(0), readEverySec_(read_every_sec), readyAfterSec_(ready_after_sec){}

AirSensor::AirSensor(int pin_dht11, const int pin_led, int pin_interrupt, void (*callback_wrapper)()) :
  BaseSensor(60,15),
  idDHT11(pin_dht11,digitalPinToInterrupt(pin_dht11),callback_wrapper),
  pinLed_(pin_led),
  temperature_(20.20),
  humidity_(60.60),
  dewPoint_(10.10),
  sensorError_(false)
{
   initSensor();
}

void AirSensor::initSensor() const
{
  pinMode(pinLed_, OUTPUT);
  digitalWrite(pinLed_, LOW);
}

void AirSensor::readSensor()
{
   unsigned long time_now_sec = millis()/1e3;
   static int errorCount;

   if( (( abs(time_now_sec - timeLastSec_) > readEverySec_) || !timeLastSec_ ) && (time_now_sec > readyAfterSec_) )
   {
      int result;
      
      timeLastSec_ = time_now_sec;
      result = idDHT11::acquireAndWait();
      
      switch (result)
      {
        case IDDHTLIB_OK: 
          temperature_ = idDHT11::getCelsius();
          humidity_ = idDHT11::getHumidity();
          dewPoint_ = idDHT11::getDewPoint();
          sensorError_ = false;
          errorCount = 0;
          break;
        case IDDHTLIB_ERROR_CHECKSUM: 
        case IDDHTLIB_ERROR_ISR_TIMEOUT: 
        case IDDHTLIB_ERROR_RESPONSE_TIMEOUT: 
        case IDDHTLIB_ERROR_DATA_TIMEOUT: 
        case IDDHTLIB_ERROR_ACQUIRING: 
        case IDDHTLIB_ERROR_DELTA: 
        case IDDHTLIB_ERROR_NOTSTARTED: 
        default: 
          ++errorCount;
          if( errorCount > ( QUARTER_SEC/readEverySec_ ) )
          {
            sensorError_ = true;
            temperature_ = 0;
            humidity_ = 0;
            dewPoint_ = -1;
          }
          break;
      }

      if(sensorError_) digitalWrite(pinLed_, HIGH);
      else digitalWrite(pinLed_, LOW);
      
      if( ( (humidity_ < stopWateringHumidity_) && (temperature_ > dewPoint_) && (temperature_ > GROUND_FROST_TEMP_DEG) ) || sensorError_) 
        shouldBeWatered_ = true;
      else shouldBeWatered_ = false;
   }
}

void AirSensor::printInfo() const
{
  Serial.print("Temperatura powietrza (oC): ");
  Serial.println(temperature_, 2);
    
  Serial.print("Wilgotnosc wzgledna powietrza (%): ");
  Serial.println(humidity_, 2);
    
  Serial.print("Punkt rosy (oC): ");
  Serial.println(dewPoint_, 2);
}

SoilSensorSegment::SoilSensorSegment(const int p1, const int p2, const int iD) :
  BaseSensor(5,1), pin_{p1,p2}, id(iD)
{
  dryness_[0] = false;
  dryness_[1] = false;
  drynessCount_[0] = 0;
  drynessCount_[1] = 0;
  initSensor();
}

void SoilSensorSegment::initSensor()const
{
  pinMode(pin_[0], INPUT);
  pinMode(pin_[1], INPUT);
}
    
void SoilSensorSegment::readSensor()
{
  unsigned int time_now_sec = millis()/1e3;

  if( (( abs(time_now_sec - timeLastSec_) > readEverySec_) || !timeLastSec_ ) && (time_now_sec > readyAfterSec_) )
  {     
    timeLastSec_ = time_now_sec;

    for(int i = 0; i<2; ++i)
    {
      if(digitalRead(pin_[i])) 
      {
        dryness_[i] = true;
        ++drynessCount_[i];
      }
      else  dryness_[i] = false;
    }

    if(dryness_[0] && dryness_[1])  shouldBeWatered_ = true;
    else  shouldBeWatered_ = false; 
  }
}
    
void SoilSensorSegment::printInfo() const
{
  Serial.print("Wilgotnosc gleby dla segmentu id=");
  Serial.print(id);
  Serial.println(": ");
  for(int i=0; i<2; ++i) {
    Serial.print("czujnik ");
    Serial.print(i+1);
    Serial.print(": ");
    if(dryness_[i]) Serial.println("sucho");
    else Serial.println("wilgotno");
  }
  for(int i=0; i<2; ++i)
  {
    Serial.print("czujnik ");
    Serial.print(i+1);
    Serial.print(" wykryl suchosc gleby ");
    Serial.print(drynessCount_[i]);
    Serial.println(" razy");
  }
}

WaterSensor::WaterSensor(const int pin_sensor, const int pin_buzzer, void (*callback_wrapper)()) :
  BaseSensor(0,0),  //irrelevant - just for the interface inheritance
  pinSensor_(pin_sensor),
  pinBuzzer_(pin_buzzer)
{
  initSensor();
  attachInterrupt(digitalPinToInterrupt(pinSensor_), callback_wrapper, CHANGE);
  readSensor();
}

void WaterSensor::initSensor() const
{
  pinMode(pinSensor_, INPUT_PULLUP);
  pinMode(pinBuzzer_, OUTPUT);

  digitalWrite(pinBuzzer_, LOW);
}

void WaterSensor::readSensor()
{
  shouldBeWatered_ = !digitalRead(pinSensor_);
  
  if(!shouldWater()) digitalWrite(pinBuzzer_, HIGH);
  else digitalWrite(pinBuzzer_, LOW);
}

void WaterSensor::printInfo() const
{
  if(shouldWater()) Serial.println("W zbiorniku jest woda");
  else Serial.println("BRAK WODY W ZBIORNIKU!");
}

Switch::Switch(const int pin) :
  BaseSensor(0,0), //irrelevant - just for the interface inheritance
  pin_(pin),
  auxFirstTime_(0),
  auxSwitchState_(0)
{
  initSensor();
  readSensor();
}

void Switch::initSensor() const
{
    pinMode(pin_, INPUT_PULLUP);
}

//oscillation resistant method
void Switch::readSensor()
{ 
  shouldBeWatered_ = !digitalRead(pin_);

  if(!auxSwitchState_ && shouldBeWatered_)
  {
    ++auxSwitchState_;
    auxFirstTime_ = millis();
  }
  else if(auxSwitchState_)
  {
    if( (millis() - auxFirstTime_ > 50) && !shouldBeWatered_ )
    {
      shouldBeWatered_ = false;
      auxSwitchState_ = 0;
    }
    else if( (millis() - auxFirstTime_ > 50) && shouldBeWatered_ )
    {
       shouldBeWatered_ = true;
    }
  }
}

