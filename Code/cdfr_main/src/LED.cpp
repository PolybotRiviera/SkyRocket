/*
 __ _            __            _        _   
/ _\ | ___   _  /__\ ___   ___| | _____| |_ 
\ \| |/ / | | |/ \/// _ \ / __| |/ / _ \ __|
_\ \   <| |_| / _  \ (_) | (__|   <  __/ |_ 
\__/_|\_\\__, \/ \_/\___/ \___|_|\_\___|\__|
         |___/                                                            
*/

/*  LED.cpp    */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "../include/LED.hpp"

LED::LED(uint16_t pin, uint16_t numPixels, neoPixelType type)
    : _pixels(numPixels, pin, type),
      _pin(pin),
      _numPixels(numPixels),
      _type(type) {
}

LED::~LED() {
    _pixels.clear();
    _pixels.show();
}

void LED::init() {
    _pixels.begin();
    _pixels.clear();
    _pixels.setBrightness(_brightness); // Max 255
    setColorRGB(0, 0, 255);
}

void LED::setColorRGB() {
    _pixels.setPixelColor(0, _pixels.Color(_red, _green, _blue));
    _pixels.show();
}

void LED::setColorRGB(int red, int green, int blue) {
    _red = red;
    _green = green;
    _blue = blue;
    _pixels.setPixelColor(0, _pixels.Color(red, green, blue));
    _pixels.show();
}

void LED::blink() {
    _pixels.setPixelColor(0, _pixels.Color(0, 0, 255));
    _pixels.show();
    vTaskDelay(pdMS_TO_TICKS(50));
    _pixels.clear();
    _pixels.show();
    vTaskDelay(pdMS_TO_TICKS(100));
    _pixels.setPixelColor(0, _pixels.Color(0, 0, 255));
    _pixels.show();
    vTaskDelay(pdMS_TO_TICKS(50));
    _pixels.clear();
    _pixels.show();
    vTaskDelay(pdMS_TO_TICKS(1000));
}
