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

#include <Arduino.h>

class Mecanum {


    public:

        Mecanum();
        void begin();
        void move(float angle, int speed, int turn);
        void rotate(int speed);
        void setSpeed(int speed) { this->speed = speed; };
        int getSpeed() { return speed; };

    private:

        static const int _LFW = 7;
        static const int _RFW = 1;
        static const int _LBW = 13;
        static const int _RBW = 3;

        static const int _LEDC_FREQ = 30000;

        int speed = 50;

};




#endif