#include<arduino.h>
#include "pumps.h"

BasePump::BasePump(const int pin_pump, const int pin_pot, const int time_per_cycle, const int iD, const Switch* pS, const WaterSensor* pWS, const AirSensor* pAS, const SoilSensorSegment* pSS) :
  pinPump_(pin_pump), pinPot_(pin_pot), 
  timePerCycle_( time_per_cycle - _DELAY_CONSTANT_SEC > 0 ? time_per_cycle : 2*_DELAY_CONSTANT_SEC ),
  id(iD), pumpState_(idle), powerOnCycleCount_(0), timeLastStartSec_(0), timeLastStopSec_(0),
  pSwitch(pS), pWaterSensor(pWS), pAirSensor(pAS), pSoilSensor(pSS)
{
  initPump();
}

void BasePump::initPump() const
{
  pinMode(pinPot_, INPUT);
  pinMode(pinPump_, OUTPUT);

  digitalWrite(pinPump_, HIGH);
}

void BasePump::startPump() const
{
  digitalWrite(pinPump_, LOW);
}

void BasePump::stopPump() const
{
  digitalWrite(pinPump_, HIGH);
}

//pump control based on internal counters. no need for greater precision
void BasePump::controlPump()
{
  unsigned long time_now_sec;

  countTimeBetweenTurnsOn();
  time_now_sec = millis()/1e3; 

  //finite state machine
  switch(pumpState_)
  {
    case idle:
      if( pWaterSensor->shouldWater() && pAirSensor->shouldWater() && ( pSoilSensor==nullptr ? true : pSoilSensor->shouldWater() ))
      {
        timeLastStartSec_ = millis()/1e3;
        pumpState_ = onAuto;
        ++powerOnCycleCount_;
        startPump();
      }
      else if( pWaterSensor->shouldWater() && pSwitch->shouldWater() )
      {
        timeLastStartSec_ = millis()/1e3;
        pumpState_ = onMan;
        startPump();
      }
      break;
    case onAuto:
      if(( pSoilSensor==nullptr ? false : !pSoilSensor->shouldWater() ) || ( abs(time_now_sec - timeLastStartSec_) > timePerCycle_ ) )
      {
        timeLastStopSec_ = millis()/1e3;
        pumpState_ = off;
        stopPump();
      }
      else if( !pWaterSensor->shouldWater() )
      {
        timeLastStopSec_ = millis()/1e3;
        pumpState_ = idle;
        stopPump();
      }
      break;
    case onMan:
      if( !pSwitch->shouldWater() || !pWaterSensor->shouldWater() )
      {
        timeLastStopSec_ = millis()/1e3;
        pumpState_ = off;
        stopPump();
      }
      break;
    case off:
      if( pWaterSensor->shouldWater() && pSwitch->shouldWater() )
      {
        timeLastStartSec_ = millis()/1e3;
        pumpState_ = onMan;
        startPump();
      }
      else if ( abs(time_now_sec - timeLastStopSec_) > timeBetweenTurnsOn_ )
      {
        pumpState_ = idle;
        stopPump();  //just to be sure
      }
      break;
  }
}

PumpSS::PumpSS(const int pin_pump, const int pin_pot, const int time_per_cycle, const int iD, const Switch* pS, const WaterSensor* pWS, const AirSensor* pAS, const SoilSensorSegment* pSS) :
  BasePump(pin_pump, pin_pot, time_per_cycle, iD, pS, pWS, pAS, pSS)
  {
    countTimeBetweenTurnsOn();
  }

void PumpSS::countTimeBetweenTurnsOn()
{  
  timeBetweenTurnsOn_ = map(analogRead(pinPot_),0,1023,2*timePerCycle_,_20_MIN_SEC-timePerCycle_);
}

void PumpSS::printInfo() const
{
  Serial.print("Minimalny czas pomiedzy uruchomieniami pompy id=");
  Serial.print(id);
  Serial.print(" [min]: ");
  Serial.println(timeBetweenTurnsOn_/60., 1);
  Serial.print("Pompa id=");
  Serial.print(id);
  Serial.print(" zostala uruchomiona ");
  Serial.print(powerOnCycleCount_);
  Serial.println(" razy");
  Serial.print("Pompa id=");
  Serial.print(id);
  switch(pumpState_)
  {
    case idle:
      Serial.println(" w stanie oczekiwania");
      break;
    case onAuto:
      Serial.println(" wlaczona automatycznie");
      break;
    case onMan:
      Serial.println(" wlaczona manualnie");
      break;
    case off:
      Serial.println(" wylaczona");
      break;
  }
}

PumpWT::PumpWT(const int pin_pump, const int pin_pot, const int time_per_cycle, const int iD, const Switch* pS, const WaterSensor* pWS, const AirSensor* pAS) :
  BasePump(pin_pump, pin_pot, time_per_cycle, iD, pS, pWS, pAS, nullptr), 
  waterPerCycle_( (timePerCycle_ - _DELAY_CONSTANT_SEC) * _WATER_L_PER_SEC )
{
  countTimeBetweenTurnsOn();
}

void PumpWT::countTimeBetweenTurnsOn()
{  
  waterPerDay_ = map(analogRead(pinPot_),0,1023,maxWaterPerDay_*100,waterPerCycle_*100);
  waterPerDay_ /= 100;

  timeBetweenTurnsOn_ = _DAY_SEC / ( waterPerDay_ / waterPerCycle_ );
}

void PumpWT::printInfo() const
{
  Serial.print("Minimalny czas pomiedzy uruchomieniami pompy id=");
  Serial.print(id);
  Serial.print(" [min]: ");
  Serial.print(timeBetweenTurnsOn_/60., 1);
  Serial.print(" -> ");
  Serial.print(waterPerDay_, 1);
  Serial.println(" [litry na dzien]");
  Serial.print("Pompa id=");
  Serial.print(id);
  Serial.print(" zostala uruchomiona ");
  Serial.print(powerOnCycleCount_);
  Serial.println(" razy");
  Serial.print("Pompa id=");
  Serial.print(id);
  switch(pumpState_)
  {
    case idle:
      Serial.println(" w stanie oczekiwania");
      break;
    case onAuto:
      Serial.println(" wlaczona automatycznie");
      break;
    case onMan:
      Serial.println(" wlaczona manualnie");
      break;
    case off:
      Serial.println(" wylaczona");
      break;
  }
}

