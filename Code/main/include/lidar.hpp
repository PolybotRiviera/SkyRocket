/*
 __ _            __            _        _   
/ _\ | ___   _  /__\ ___   ___| | _____| |_ 
\ \| |/ / | | |/ \/// _ \ / __| |/ / _ \ __|
_\ \   <| |_| / _  \ (_) | (__|   <  __/ |_ 
\__/_|\_\\__, \/ \_/\___/ \___|_|\_\___|\__|
         |___/                                                            
*/

/*  Lidar.hpp    */

#ifndef LIDAR_HPP
#define LIDAR_HPP

#include <Arduino.h>

class Lidar {
public:
    Lidar(HardwareSerial& serialPort, int rxPin, long baudRate = 230400);
    ~Lidar();

    bool init();  // Initialize the LIDAR
    bool update(); // Process LIDAR data, returns true if valid packet processed

    // Data accessors
    uint16_t getSpeed() const { return speed; }
    float getStartAngle() const { return startAngle * 0.01f; }  // Degrees
    float getEndAngle() const { return endAngle * 0.01f; }      // Degrees
    uint16_t getTimestamp() const { return timestamp; }
    uint16_t getDistance(int index) const { return (index >= 0 && index < 12) ? distances[index] : 0; }
    byte getIntensity(int index) const { return (index >= 0 && index < 12) ? intensities[index] : 0; }
    bool tooClose();

private:
    HardwareSerial& serial;  // Reference to Serial port
    int rxPin;              // RX pin for serial communication

    // LIDAR data
    uint16_t speed;
    uint16_t startAngle;
    uint16_t endAngle;
    uint16_t timestamp;
    uint16_t distances[12];  // 12 distance points
    byte intensities[12];    // 12 intensity points
    float minValidDist = 60.0f; 
    float thresholdDist = 200.0f; 

    // Buffer and constants
    static const int BUFFER_SIZE = 47;  // 2 header + 43 data + 2 CRC
    byte buffer[BUFFER_SIZE];

    // CRC table
    static const uint8_t CrcTable[256];

    uint8_t calculateCrc8(uint8_t* data, uint8_t len);  // CRC calculation
};

#endif