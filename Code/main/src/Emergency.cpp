/*
 __ _            __            _        _   
/ _\ | ___   _  /__\ ___   ___| | _____| |_ 
\ \| |/ / | | |/ \/// _ \ / __| |/ / _ \ __|
_\ \   <| |_| / _  \ (_) | (__|   <  __/ |_ 
\__/_|\_\\__, \/ \_/\___/ \___|_|\_\___|\__|
         |___/                                                            
*/

/*  Emergency.cpp    */

#include "../include/Emergency.hpp"
#include <Arduino.h>

Emergency::Emergency(){}

void Emergency::begin(){
    pinMode(_MOSFET_PIN, OUTPUT);
    digitalWrite(_MOSFET_PIN, HIGH);
}

void Emergency::activate(){
    digitalWrite(_MOSFET_PIN, HIGH);
}

void Emergency::stop(){
    digitalWrite(_MOSFET_PIN, LOW);
}