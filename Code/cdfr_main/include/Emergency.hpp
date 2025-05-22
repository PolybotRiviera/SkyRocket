/*
 __ _            __            _        _   
/ _\ | ___   _  /__\ ___   ___| | _____| |_ 
\ \| |/ / | | |/ \/// _ \ / __| |/ / _ \ __|
_\ \   <| |_| / _  \ (_) | (__|   <  __/ |_ 
\__/_|\_\\__, \/ \_/\___/ \___|_|\_\___|\__|
         |___/                                                            
*/

/*  Emergency.hpp    */

#ifndef EMERGENCY_HPP
#define EMERGENCY_HPP


#include <Arduino.h>


class Emergency {

    public:

        Emergency();
        void begin();
        void activate();
        void stop();

    private:

        static const int _MOSFET_PIN = 4;
};


#endif


