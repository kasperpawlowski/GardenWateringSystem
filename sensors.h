#ifndef SENSORS_H
#define SENSORS_H

#include "idDHT11.h"

const int QUARTER_SEC = 3600/4;
const double GROUND_FROST_TEMP_DEG = 5;

class BaseSensor
{
  private:
    bool shouldBeWatered_;
    unsigned long timeLastSec_;
    const int readEverySec_;
    const int readyAfterSec_;
    virtual void initSensor() const = 0;
  public:
    BaseSensor(const int read_every_sec, const int ready_after_sec);
    virtual ~BaseSensor() {};
    virtual void readSensor() = 0;
    bool shouldWater() const { return shouldBeWatered_; }
    virtual void printInfo() const = 0;

    friend class AirSensor;
    friend class SoilSensorSegment;
    friend class WaterSensor;
    friend class Switch;
};

class AirSensor : public BaseSensor, public idDHT11
{
  private:
    const int pinLed_;
    double temperature_;
    double humidity_;
    double dewPoint_;
    bool sensorError_;
    const double stopWateringHumidity_ = 80;
    void initSensor() const;
  public:
    AirSensor() = delete;
    AirSensor(int pin_dht11, const int pin_led, int pin_interrupt, void (*callback_wrapper)());
    ~AirSensor() {};
    void readSensor();
    void printInfo() const;
};

class SoilSensorSegment : public BaseSensor
{
  private:
    const int pin_[2];
    bool dryness_[2];
    unsigned int drynessCount_[2];
    const int id;
    void initSensor() const;
  public:
    SoilSensorSegment() = delete;
    SoilSensorSegment(const int p1, const int p2, const int iD);
    ~SoilSensorSegment() {};
    void readSensor();
    void printInfo() const;
};

class WaterSensor : public BaseSensor
{
  private:
    const int pinSensor_;
    const int pinBuzzer_;
    void initSensor() const;
    static void readSensorWrapper();
  public:
    WaterSensor() = delete;
    WaterSensor(const int pin_sensor, const int pin_buzzer, void (*callback_wrapper)());
    ~WaterSensor() {};
    void readSensor();
    void printInfo() const;
};

class Switch : public BaseSensor
{
  private:
    const int pin_;
    unsigned long auxFirstTime_;
    int auxSwitchState_;
    bool auxSwitchOn_;
    void initSensor() const;
  public:
    Switch() = delete;
    Switch(const int pin);
    ~Switch() {};
    void readSensor();
    void printInfo() const {};
};

#endif
