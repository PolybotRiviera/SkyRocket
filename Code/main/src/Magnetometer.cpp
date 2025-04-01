/*
 __ _            __            _        _   
/ _\ | ___   _  /__\ ___   ___| | _____| |_ 
\ \| |/ / | | |/ \/// _ \ / __| |/ / _ \ __|
_\ \   <| |_| / _  \ (_) | (__|   <  __/ |_ 
\__/_|\_\\__, \/ \_/\___/ \___|_|\_\___|\__|
         |___/                                                            
*/

/*  Magnetometer.hpp    */

#include "../include/Magnetometer.hpp"

Magnetometer::Magnetometer(int i2cAddress) 
    : mag(i2cAddress), 
      magX_offset(0),
      magY_offset(0),
      calibrated(false),
      kp(1.0),         
      ki(0.0),
      kd(0.0),
      integral(0.0),
      lastError(0.0),
      targetHeading(100.0),
      lastTime(0) {
}
Magnetometer::~Magnetometer() {
}

void Magnetometer::begin(TwoWire& wire, int sda, int scl) {
    wire.begin(sda, scl);
}

bool Magnetometer::initialize() {
    if (!mag.begin()) {
        return false;
    }
    return true;
}

bool Magnetometer::calibrate(int timeout) {
    
    float magX_min = 1000.0;
    float magX_max = -1000.0;
    float magY_min = 1000.0;
    float magY_max = -1000.0;
    
    unsigned long startTime = millis();
    
    while (millis() - startTime < timeout) {
        sensors_event_t event;
        mag.getEvent(&event);
        
        magX_min = min(magX_min, event.magnetic.x);
        magX_max = max(magX_max, event.magnetic.x);
        magY_min = min(magY_min, event.magnetic.y);
        magY_max = max(magY_max, event.magnetic.y);
        
        vTaskDelay(50);
    }
    
    magX_offset = (magX_max + magX_min) / 2.0;
    magY_offset = (magY_max + magY_min) / 2.0;
        
    calibrated = true;
    return calibrated;
}

float Magnetometer::getHeading() {
    sensors_event_t event;
    mag.getEvent(&event);
    
    float calibratedX = event.magnetic.x - magX_offset;
    float calibratedY = event.magnetic.y - magY_offset;
    
    float heading = atan2(calibratedY, calibratedX);
    heading = heading * 180.0 / PI;
    
    if (heading < 0) {
        heading += 360.0;
    }
    
    return heading;
}

void Magnetometer::readRaw(float& x, float& y, float& z) {
    sensors_event_t event;
    mag.getEvent(&event);
    x = event.magnetic.x;
    y = event.magnetic.y;
    z = event.magnetic.z;
}

void Magnetometer::setPIDTunings(float kp, float ki, float kd) {
    this->kp = kp;
    this->ki = ki;
    this->kd = kd;
}

void Magnetometer::setTargetHeading(float target) {
    targetHeading = target;
}

float Magnetometer::computePID(float currentHeading) {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0;
    lastTime = now;

    float error = targetHeading - currentHeading;

    if (error > 180) {
        error -= 360;
    } else if (error < -180) {
        error += 360;
    }

    float pTerm = kp * error;

    integral += error * dt;
    float iTerm = ki * integral;

    float derivative = (error - lastError) / dt;
    float dTerm = kd * derivative;

    float output = pTerm + iTerm + dTerm;
    lastError = error;

    //output = constrain(output, -50, 50);

    return output;
}