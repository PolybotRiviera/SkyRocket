/*
 __ _            __            _        _   
/ _\ | ___   _  /__\ ___   ___| | _____| |_ 
\ \| |/ / | | |/ \/// _ \ / __| |/ / _ \ __|
_\ \   <| |_| / _  \ (_) | (__|   <  __/ |_ 
\__/_|\_\\__, \/ \_/\___/ \___|_|\_\___|\__|
         |___/                                                            
*/

/*  Hedgehog.hpp    */

#ifndef HEDGEHOG_HPP
#define HEDGEHOG_HPP

#include <Arduino.h>

class Hedgehog {
public:
    Hedgehog(HardwareSerial& serialPort, long baudRate = 115200);
    ~Hedgehog();

    bool init();         // Initialize the Hedgehog
    void update();       // Process incoming serial data
    void processData();  // Handle position data and update current coordinates

    // Position data accessors
    long getX() const { return hedgehog_x; }
    long getY() const { return hedgehog_y; }
    long getZ() const { return hedgehog_z; }
    bool isPositionUpdated() const { return hedgehog_pos_updated; }

    // IMU data accessors
    int16_t getAccX() const { return imu_acc_x; }
    int16_t getAccY() const { return imu_acc_y; }
    int16_t getAccZ() const { return imu_acc_z; }
    int16_t getGyroX() const { return imu_gyro_x; }
    int16_t getGyroY() const { return imu_gyro_y; }
    int16_t getGyroZ() const { return imu_gyro_z; }
    int64_t getRawTimestamp() const { return imu_raw_timestamp; }

    void setAngle(float newAngle) { angle = newAngle; }  // Set the angle for PID control
    float getAngle() const { return angle; }  // Get the current angle


private:
    HardwareSerial& serial;  // Reference to Serial port

    float angle = 0;  // Current angle between the robot and the x and y axis of the beacons

    float kp;  // PID parameters
    float ki;
    float kd;
    float integral;
    float lastError;
    float targetHeading;
    unsigned long lastTime;
    int sampleTime = 100;  // ms
    float correction = 0;

    // Position data
    long hedgehog_x, hedgehog_y, hedgehog_z;
    int hedgehog_pos_updated;

    // IMU data
    int16_t imu_acc_x, imu_acc_y, imu_acc_z;
    int16_t imu_gyro_x, imu_gyro_y, imu_gyro_z;
    int64_t imu_raw_timestamp;

    // Buffer and packet handling
    static const byte HEDGEHOG_BUF_SIZE = 80;
    static const byte HEDGEHOG_CM_DATA_SIZE = 0x10;
    static const byte HEDGEHOG_MM_DATA_SIZE = 0x16;
    static const byte HEDGEHOG_RAW_IMU_DATA_SIZE = 0x20;
    byte hedgehog_serial_buf[HEDGEHOG_BUF_SIZE];
    byte hedgehog_serial_buf_ofs;

    // Datagram IDs
    static const unsigned int POSITION_DATAGRAM_ID = 0x0001;
    static const unsigned int POSITION_DATAGRAM_HIGHRES_ID = 0x0011;
    static const unsigned int RAW_IMU_DATAGRAM_ID = 0x0003;
    static const unsigned int NT_POSITION_DATAGRAM_HIGHRES_ID = 0x0081;
    static const unsigned int NT_RAW_IMU_DATAGRAM_ID = 0x0083;
    unsigned int hedgehog_data_id;

    // Unions for data parsing
    union uni_8x2_16 {
        byte b[2];
        unsigned int w;
        int wi;
    };
    union uni_8x4_32 {
        byte b[4];
        float f;
        unsigned long v32;
        long vi32;
    };
    union uni_8x8_64 {
        byte b[8];
        int64_t vi64;
    };

    void setCrc16(byte* buf, byte size);  // CRC calculation
};

#endif