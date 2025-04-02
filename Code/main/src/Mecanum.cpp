/*
 __ _            __            _        _   
/ _\ | ___   _  /__\ ___   ___| | _____| |_ 
\ \| |/ / | | |/ \/// _ \ / __| |/ / _ \ __|
_\ \   <| |_| / _  \ (_) | (__|   <  __/ |_ 
\__/_|\_\\__, \/ \_/\___/ \___|_|\_\___|\__|
         |___/                                                            
*/

/*  Mecanum.cpp    */

#include "../include/Mecanum.hpp"
#include "../include/Magnetometer.hpp"
#include <Arduino.h>

Mecanum::Mecanum(){}

void Mecanum::begin(){

    ledcAttach(_LFW, _LEDC_FREQ, 8);
    ledcAttach(_RFW, _LEDC_FREQ, 8);
    ledcAttach(_LBW, _LEDC_FREQ, 8);
    ledcAttach(_RBW, _LEDC_FREQ, 8);

    ledcWrite(_LFW, 125);
    ledcWrite(_RFW, 125);
    ledcWrite(_LBW, 125);
    ledcWrite(_RBW, 125);
}

void Mecanum::move(float angle, int speed, int turn){
    float rad = angle * PI / 180.0;

    speed = map(speed, 0, 100, 0, 185);
    speed -= abs(turn);
    speed = constrain(speed, 0, 185);

    int v1 = speed * sin(rad + PI / 4) + turn;
    int v2 = speed * cos(rad + PI / 4) - turn;
    int v3 = speed * cos(rad + PI / 4) + turn;
    int v4 = speed * sin(rad + PI / 4) - turn;
    
    v1 = constrain(125 + v1, 0, 255);
    v2 = constrain(125 - v2, 0, 255);
    v3 = constrain(125 + v3, 0, 255);
    v4 = constrain(125 - v4, 0, 255);

    ledcWrite(_LFW, v1);
    ledcWrite(_RFW, v2);
    ledcWrite(_LBW, v3);
    ledcWrite(_RBW, v4);
}

void Mecanum::rotate(int speed){
    speed = map(speed, -100, 100, -125, 130);
    ledcWrite(_LFW, 125 + speed);
    ledcWrite(_RFW, 125 + speed);
    ledcWrite(_LBW, 125 + speed);
    ledcWrite(_RBW, 125 + speed);
}
