#ifndef PUMPS_H
#define PUMPS_H

#include "sensors.h"

enum State {idle, onAuto, onMan, off};
const int _MAX_WATER_PER_DAY_L = 50;
const double _WATER_L_PER_SEC = 0.05;
const int _DELAY_CONSTANT_SEC = 10;
const long _DAY_SEC = 86400;
const int _20_MIN_SEC = 1200;

class BasePump
{
  private:
    const int pinPump_;
    const int pinPot_;
    enum State pumpState_;
    int timePerCycle_;
    unsigned long timeBetweenTurnsOn_;
    unsigned long timeLastStartSec_;
    unsigned long timeLastStopSec_;
    unsigned int powerOnCycleCount_;
    Switch const* pSwitch;
    WaterSensor const* pWaterSensor;
    AirSensor const* pAirSensor;
    SoilSensorSegment const* pSoilSensor;
    const int id;
    void initPump() const;
    virtual void countTimeBetweenTurnsOn() = 0;
  public:
    BasePump(const int pin_pump, const int pin_pot, const int iD, const int time_per_cycle, const Switch* pS, const WaterSensor* pWS, const AirSensor* pAS, const SoilSensorSegment* pSS);
    virtual ~BasePump() {};
    void controlPump();
    void startPump() const;
    void stopPump() const;
    virtual void printInfo() const = 0;

    friend class PumpSS;
    friend class PumpWT;
};

//class for pump controlled by soil sensors
//potentiometer used for setting the shortest period of time between turning on (from 2*timePerCycle_ up to about 20 min)
class PumpSS : public BasePump
{
  private:
    void countTimeBetweenTurnsOn();
  public:
    PumpSS(const int pin_pump, const int pin_pot, const int iD, const int time_per_cycle, const Switch* pS, const WaterSensor* pWS, const AirSensor* pAS, const SoilSensorSegment* pSS);
    ~PumpSS() {};
    void printInfo() const;
};

//class for pump controlled by timer
//potentiometer used for changing max water per day per pump (from amount of water per one cycle up to _MAX_WATER_PER_DAY_L)
class PumpWT : public BasePump
{
  private:
    const double waterPerCycle_;
    const int maxWaterPerDay_ = _MAX_WATER_PER_DAY_L;
    double waterPerDay_;
    void countTimeBetweenTurnsOn();
  public:
    PumpWT(const int pin_pump, const int pin_pot, const int iD, const int time_per_cycle, const Switch* pS, const WaterSensor* pWS, const AirSensor* pAS);
    ~PumpWT() {};
    void printInfo() const;
};
#endif
