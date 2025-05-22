/*
 __ _            __            _        _
/ _\ | ___   _  /__\ ___   ___| | _____| |_
\ \| |/ / | | |/ \/// _ \ / __| |/ / _ \ __|
_\ \   <| |_| / _  \ (_) | (__|   <  __/ |_
\__/_|\_\\__, \/ \_/\___/ \___|_|\_\___|\__|
         |___/
*/

/*  Mecanum.hpp    */

#ifndef MECANUM_HPP
#define MECANUM_HPP
#include "driver/ledc.h"

#include <Arduino.h>

class Mecanum
{

public:
    Mecanum();
    void begin();
    void move(float angle, int speed, int turn);
    void rotate(int speed);
    void setSpeed(int speed) { this->speed = speed; };
    int getSpeed() { return speed; };
    void setAngle(float angle) { this->angle = angle; };
    float getAngle() { return angle; };
    void setTurn(int turn) { this->turn = turn; };
    int getTurn() { return turn; };
    void setState(int state) { this->state = state; };
    int getState() { return state; };
    void stop()
    {
        ledcWrite(_LFW, 125);
        ledcWrite(_RFW, 125);
        ledcWrite(_LBW, 125);
        ledcWrite(_RBW, 125);
    }

private:
    static const int _LFW = 7;
    static const int _RFW = 1;
    static const int _LBW = 13;
    static const int _RBW = 3;

    static const int _LEDC_FREQ = 30000;

    int speed = 50;
    float angle = 0;
    int turn = 0;
    int state = 0;
    // ledc_timer_config_t ledc_timer;
    // ledc_channel_config_t _LFW_CHANNEL;
    // ledc_channel_config_t _RFW_CHANNEL;
    // ledc_channel_config_t _LBW_CHANNEL;
    // ledc_channel_config_t _RBW_CHANNEL;
};

#endif