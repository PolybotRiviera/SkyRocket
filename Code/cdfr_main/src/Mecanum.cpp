/*
 __ _            __            _        _
/ _\ | ___   _  /__\ ___   ___| | _____| |_
\ \| |/ / | | |/ \/// _ \ / __| |/ / _ \ __|
_\ \   <| |_| / _  \ (_) | (__|   <  __/ |_
\__/_|\_\\__, \/ \_/\___/ \___|_|\_\___|\__|
         |___/
*/

/*  Mecanum.cpp    */

#include <Arduino.h>
#include "driver/ledc.h"
#include "../include/Mecanum.hpp"
#include "../include/Magnetometer.hpp"

Mecanum::Mecanum() {}

void Mecanum::begin()
{
    // ledc_timer = {
    //     .speed_mode = ledc_mode_t::LEDC_LOW_SPEED_MODE,
    //     .bit_num = LEDC_TIMER_8_BIT,
    //     .timer_num = LEDC_TIMER_0,
    //     .freq_hz = 200};
    // ledc_timer_config(&ledc_timer);
    // _LFW_CHANNEL = {
    //     .gpio_num = _LFW,
    //     .speed_mode = ledc_mode_t::LEDC_LOW_SPEED_MODE,
    //     .channel = LEDC_CHANNEL_0,
    //     .intr_type = LEDC_INTR_DISABLE,
    //     .timer_sel = LEDC_TIMER_0,
    //     .duty = 128,
    //     .hpoint = 0
    // };
    // ledc_channel_config(&_LFW_CHANNEL);
    // _RFW_CHANNEL = {
    //     .gpio_num = _RFW,
    //     .speed_mode = ledc_mode_t::LEDC_LOW_SPEED_MODE,
    //     .channel = LEDC_CHANNEL_1,
    //     .intr_type = LEDC_INTR_DISABLE,
    //     .timer_sel = LEDC_TIMER_0,
    //     .duty = 128,
    //     .hpoint = 0
    // };
    // ledc_channel_config(&_RFW_CHANNEL);
    // _LBW_CHANNEL = {
    //     .gpio_num = _LBW,
    //     .speed_mode = ledc_mode_t::LEDC_LOW_SPEED_MODE,
    //     .channel = LEDC_CHANNEL_2,
    //     .intr_type = LEDC_INTR_DISABLE,
    //     .timer_sel = LEDC_TIMER_0,
    //     .duty = 128,
    //     .hpoint = 0
    // };
    // ledc_channel_config(&_LBW_CHANNEL);
    // _RBW_CHANNEL = {
    //     .gpio_num = _RBW,
    //     .speed_mode = ledc_mode_t::LEDC_LOW_SPEED_MODE,
    //     .channel = LEDC_CHANNEL_3,
    //     .intr_type = LEDC_INTR_DISABLE,
    //     .timer_sel = LEDC_TIMER_0,
    //     .duty = 128,
    //     .hpoint = 0
    // };
    // ledc_channel_config(&_RBW_CHANNEL);

    // ledcAttach(_LFW, _LEDC_FREQ, 8);

    // ledcAttach(_RFW, _LEDC_FREQ, 8);
    // ledcAttach(_LBW, _LEDC_FREQ, 8);
    // ledcAttach(_RBW, _LEDC_FREQ, 8);
    ledcSetup(LEDC_CHANNEL_0, _LEDC_FREQ, 8);
    ledcAttachPin(_LFW, LEDC_CHANNEL_0);

    ledcSetup(LEDC_CHANNEL_1, _LEDC_FREQ, 8);
    ledcAttachPin(_RFW, LEDC_CHANNEL_1);

    ledcSetup(LEDC_CHANNEL_2, _LEDC_FREQ, 8);
    ledcAttachPin(_LBW, LEDC_CHANNEL_2);

    ledcSetup(LEDC_CHANNEL_3, _LEDC_FREQ, 8);
    ledcAttachPin(_RBW, LEDC_CHANNEL_3);

    ledcWrite(_LFW, 125);
    ledcWrite(_RFW, 125);
    ledcWrite(_LBW, 125);
    ledcWrite(_RBW, 125);
}

void Mecanum::move(float angle, int speed, int turn)
{
    float rad = angle * PI / 180.0;

    speed = map(speed, 0, 100, 0, 185);
    speed -= abs(turn);
    speed = constrain(speed, 0, 185);
    // turn = map(turn, -100, 100, -255, 255);

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

void Mecanum::rotate(int speed)
{
    speed = map(speed, -100, 100, -125, 130);
    ledcWrite(_LFW, 125 + speed);
    ledcWrite(_RFW, 125 + speed);
    ledcWrite(_LBW, 125 + speed);
    ledcWrite(_RBW, 125 + speed);
}
