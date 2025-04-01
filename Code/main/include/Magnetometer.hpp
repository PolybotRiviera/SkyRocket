/*
 __ _            __            _        _   
/ _\ | ___   _  /__\ ___   ___| | _____| |_ 
\ \| |/ / | | |/ \/// _ \ / __| |/ / _ \ __|
_\ \   <| |_| / _  \ (_) | (__|   <  __/ |_ 
\__/_|\_\\__, \/ \_/\___/ \___|_|\_\___|\__|
         |___/                                                            
*/

/*  Magnetometer.cpp    */
#ifndef MAGNETOMETER_HPP
#define MAGNETOMETER_HPP

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LIS2MDL.h>
#include "USB.h"

class Magnetometer {
private:
    Adafruit_LIS2MDL mag;
    float magX_offset;
    float magY_offset;
    bool calibrated;

    float kp;  
    float ki;     
    float kd;   
    float integral; 
    float lastError;     
    float targetHeading;  
    unsigned long lastTime;
    float correction = 0;

public:
    Magnetometer(int i2cAddress = 12345);
    ~Magnetometer();

    void begin(TwoWire& wire = Wire, int sda = 38, int scl = 39);
    bool initialize();
    bool calibrate(int timeout = 5000);
    float getHeading();
    void readRaw(float& x, float& y, float& z);

    void setPIDTunings(float kp, float ki, float kd);
    void setTargetHeading(float target);
    float computePID(float currentHeading);
    void setCorrection(float correction) { this->correction = correction; }
    float getCorrection() { return correction; }
};

#endif