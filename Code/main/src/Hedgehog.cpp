/*
 __ _            __            _        _   
/ _\ | ___   _  /__\ ___   ___| | _____| |_ 
\ \| |/ / | | |/ \/// _ \ / __| |/ / _ \ __|
_\ \   <| |_| / _  \ (_) | (__|   <  __/ |_ 
\__/_|\_\\__, \/ \_/\___/ \___|_|\_\___|\__|
         |___/                                                            
*/

/*  Hedgehog.cpp    */

#include <Arduino.h>
#include "../include/Hedgehog.hpp"

Hedgehog::Hedgehog(HardwareSerial& serialPort, long baudRate)
    : serial(serialPort),
      hedgehog_x(0), hedgehog_y(0), hedgehog_z(0),
      imu_acc_x(0), imu_acc_y(0), imu_acc_z(0),
      imu_gyro_x(0), imu_gyro_y(0), imu_gyro_z(0),
      imu_raw_timestamp(0),
      hedgehog_serial_buf_ofs(0),
      hedgehog_data_id(0) {
}

Hedgehog::~Hedgehog() {
}

bool Hedgehog::init() {
    serial.begin(115200);  // Default baud rate from original code
    serial.println("Init");
    hedgehog_serial_buf_ofs = 0;
    return true;  // No hardware check, so assume success
}

void Hedgehog::update() {
    int incoming_byte;
    int total_received_in_loop = 0;
    bool packet_received = false;
    byte packet_size;
    uni_8x2_16 un16;
    uni_8x4_32 un32;
    uni_8x8_64 un64;

    while (serial.available() > 0) {
        if (hedgehog_serial_buf_ofs >= HEDGEHOG_BUF_SIZE) {
            hedgehog_serial_buf_ofs = 0;  // Buffer overflow, reset
            break;
        }
        total_received_in_loop++;
        if (total_received_in_loop > 100) break;  // Limit data without header

        incoming_byte = serial.read();
        bool good_byte = false;

        switch (hedgehog_serial_buf_ofs) {
            case 0:
                good_byte = (incoming_byte == 0xff);
                break;
            case 1:
                good_byte = (incoming_byte == 0x47);
                break;
            case 2:
                good_byte = true;
                break;
            case 3:
                hedgehog_data_id = (((unsigned int)incoming_byte) << 8) + hedgehog_serial_buf[2];
                good_byte = (hedgehog_data_id == POSITION_DATAGRAM_ID) ||
                            (hedgehog_data_id == POSITION_DATAGRAM_HIGHRES_ID) ||
                            (hedgehog_data_id == RAW_IMU_DATAGRAM_ID) ||
                            (hedgehog_data_id == NT_POSITION_DATAGRAM_HIGHRES_ID) ||
                            (hedgehog_data_id == NT_RAW_IMU_DATAGRAM_ID);
                break;
            case 4:
                switch (hedgehog_data_id) {
                    case POSITION_DATAGRAM_ID:
                        good_byte = (incoming_byte == HEDGEHOG_CM_DATA_SIZE);
                        break;
                    case POSITION_DATAGRAM_HIGHRES_ID:
                        good_byte = (incoming_byte == HEDGEHOG_MM_DATA_SIZE);
                        break;
                    case RAW_IMU_DATAGRAM_ID:
                        good_byte = (incoming_byte == HEDGEHOG_RAW_IMU_DATA_SIZE);
                        break;
                    case NT_POSITION_DATAGRAM_HIGHRES_ID:
                    case NT_RAW_IMU_DATAGRAM_ID:
                        good_byte = true;
                        break;
                }
                break;
            default:
                good_byte = true;
                break;
        }

        if (!good_byte) {
            hedgehog_serial_buf_ofs = 0;  // Reset on bad byte
            continue;
        }

        hedgehog_serial_buf[hedgehog_serial_buf_ofs++] = incoming_byte;
        if (hedgehog_serial_buf_ofs > 5) {
            packet_size = 7 + hedgehog_serial_buf[4];
            if (hedgehog_serial_buf_ofs == packet_size) {
                packet_received = true;
                hedgehog_serial_buf_ofs = 0;
                break;
            }
        }
    }

    if (packet_received) {
        setCrc16(hedgehog_serial_buf, packet_size);
        if (hedgehog_serial_buf[packet_size] == 0 && hedgehog_serial_buf[packet_size + 1] == 0) {
            switch (hedgehog_data_id) {
                case POSITION_DATAGRAM_ID:
                    un16.b[0] = hedgehog_serial_buf[9];
                    un16.b[1] = hedgehog_serial_buf[10];
                    hedgehog_x = 10 * long(un16.wi);

                    un16.b[0] = hedgehog_serial_buf[11];
                    un16.b[1] = hedgehog_serial_buf[12];
                    hedgehog_y = 10 * long(un16.wi);

                    un16.b[0] = hedgehog_serial_buf[13];
                    un16.b[1] = hedgehog_serial_buf[14];
                    hedgehog_z = 10 * long(un16.wi);

                    break;

                case POSITION_DATAGRAM_HIGHRES_ID:
                case NT_POSITION_DATAGRAM_HIGHRES_ID:
                    {  // Add scope to isolate 'ofs'
                        byte ofs = (hedgehog_data_id == NT_POSITION_DATAGRAM_HIGHRES_ID) ? 13 : 9;

                        un32.b[0] = hedgehog_serial_buf[ofs + 0];
                        un32.b[1] = hedgehog_serial_buf[ofs + 1];
                        un32.b[2] = hedgehog_serial_buf[ofs + 2];
                        un32.b[3] = hedgehog_serial_buf[ofs + 3];
                        hedgehog_x = un32.vi32;

                        un32.b[0] = hedgehog_serial_buf[ofs + 4];
                        un32.b[1] = hedgehog_serial_buf[ofs + 5];
                        un32.b[2] = hedgehog_serial_buf[ofs + 6];
                        un32.b[3] = hedgehog_serial_buf[ofs + 7];
                        hedgehog_y = un32.vi32;

                        un32.b[0] = hedgehog_serial_buf[ofs + 8];
                        un32.b[1] = hedgehog_serial_buf[ofs + 9];
                        un32.b[2] = hedgehog_serial_buf[ofs + 10];
                        un32.b[3] = hedgehog_serial_buf[ofs + 11];
                        hedgehog_z = un32.vi32;
                    }
                    break;

                case RAW_IMU_DATAGRAM_ID:
                case NT_RAW_IMU_DATAGRAM_ID:
                    un16.b[0] = hedgehog_serial_buf[5];
                    un16.b[1] = hedgehog_serial_buf[6];
                    imu_acc_x = un16.wi;

                    un16.b[0] = hedgehog_serial_buf[7];
                    un16.b[1] = hedgehog_serial_buf[8];
                    imu_acc_y = un16.wi;

                    un16.b[0] = hedgehog_serial_buf[9];
                    un16.b[1] = hedgehog_serial_buf[10];
                    imu_acc_z = un16.wi;

                    un16.b[0] = hedgehog_serial_buf[11];
                    un16.b[1] = hedgehog_serial_buf[12];
                    imu_gyro_x = un16.wi;

                    un16.b[0] = hedgehog_serial_buf[13];
                    un16.b[1] = hedgehog_serial_buf[14];
                    imu_gyro_y = un16.wi;

                    un16.b[0] = hedgehog_serial_buf[15];
                    un16.b[1] = hedgehog_serial_buf[16];
                    imu_gyro_z = un16.wi;

                    if (hedgehog_data_id == RAW_IMU_DATAGRAM_ID) {
                        un32.b[0] = hedgehog_serial_buf[29];
                        un32.b[1] = hedgehog_serial_buf[30];
                        un32.b[2] = hedgehog_serial_buf[31];
                        un32.b[3] = hedgehog_serial_buf[32];
                        imu_raw_timestamp = un32.vi32;
                    } else {
                        un64.b[0] = hedgehog_serial_buf[29];
                        un64.b[1] = hedgehog_serial_buf[30];
                        un64.b[2] = hedgehog_serial_buf[31];
                        un64.b[3] = hedgehog_serial_buf[32];
                        un64.b[4] = hedgehog_serial_buf[33];
                        un64.b[5] = hedgehog_serial_buf[34];
                        un64.b[6] = hedgehog_serial_buf[35];
                        un64.b[7] = hedgehog_serial_buf[36];
                        imu_raw_timestamp = un64.vi64;
                    }

                    break;
            }
        }
    }
}

void Hedgehog::setCrc16(byte* buf, byte size) {
    uni_8x2_16 sum;
    byte shift_cnt;
    byte byte_cnt;

    sum.w = 0xffffU;

    for (byte_cnt = size; byte_cnt > 0; byte_cnt--) {
        sum.w = (unsigned int)((sum.w / 256U) * 256U + ((sum.w % 256U) ^ (buf[size - byte_cnt])));
        for (shift_cnt = 0; shift_cnt < 8; shift_cnt++) {
            if ((sum.w & 0x1) == 1) sum.w = (unsigned int)((sum.w >> 1) ^ 0xa001U);
            else sum.w >>= 1;
        }
    }

    buf[size] = sum.b[0];
    buf[size + 1] = sum.b[1];  // Little endian
}

float Hedgehog::computePID(float distanceToTarget){
    
    unsigned long currentTime = millis();
    float deltaTime = (currentTime - lastTime) / 1000.0;

    if (lastTime == 0 || deltaTime <= 0) {
        lastTime = currentTime;
        lastDistanceToTarget = distanceToTarget;
        integral = 0;
        return kp * distanceToTarget;
    }

    const float maxIntegral = 125.0;
    integral += distanceToTarget * deltaTime;
    integral = constrain(integral, -maxIntegral, maxIntegral);

    float derivative = (distanceToTarget - lastDistanceToTarget) / deltaTime;

    float output = kp * distanceToTarget + ki * integral + kd * derivative;

    lastDistanceToTarget = distanceToTarget;
    lastTime = currentTime;
    output = constrain(output, -125, 130); 
    return output;
}