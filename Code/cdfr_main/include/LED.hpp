/*
 __ _            __            _        _   
/ _\ | ___   _  /__\ ___   ___| | _____| |_ 
\ \| |/ / | | |/ \/// _ \ / __| |/ / _ \ __|
_\ \   <| |_| / _  \ (_) | (__|   <  __/ |_ 
\__/_|\_\\__, \/ \_/\___/ \___|_|\_\___|\__|
         |___/                                                            
*/

/*  LED.hpp    */

#ifndef LED_HPP
#define LED_HPP

#include <Adafruit_NeoPixel.h>

class LED {
public:
    LED(uint16_t pin, uint16_t numPixels, neoPixelType type);
    ~LED();

    void init();
    void setColorRGB();
    void setColorRGB(int red, int green, int blue);
    void setBrightness(int brightness) { _pixels.setBrightness(brightness); }
    void blink();


private:
    Adafruit_NeoPixel _pixels;
    uint16_t _pin;
    uint16_t _numPixels;
    neoPixelType _type;
    int _brightness = 10;
    int _red;
    int _green;
    int _blue;
};

#endif