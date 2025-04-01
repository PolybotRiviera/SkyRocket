/*
 __ _            __            _        _   
/ _\ | ___   _  /__\ ___   ___| | _____| |_ 
\ \| |/ / | | |/ \/// _ \ / __| |/ / _ \ __|
_\ \   <| |_| / _  \ (_) | (__|   <  __/ |_ 
\__/_|\_\\__, \/ \_/\___/ \___|_|\_\___|\__|
         |___/                                                            
*/

/*  main.ino    */


#include <Arduino.h>
#include <unordered_map>

#include "include/Mecanum.hpp"
#include "include/Emergency.hpp"
#include "include/LED.hpp"
#include "include/BLE.hpp"
#include "include/Magnetometer.hpp"
#include "USB.h"

#define DEBUG true
#define DEBUG_PRINTLN(x) if (DEBUG) USBSerial.println(x)

USBCDC USBSerial;
Mecanum mecanum;
Emergency emergency;
LED led(21, 1, NEO_GRB + NEO_KHZ800);
BLE ble;
Magnetometer mag;

TaskHandle_t blinkTaskHandle; 
TaskHandle_t calibrateTaskHandle;
TaskHandle_t moveYawCompensatedTaskHandle;



enum COMMAND : uint8_t {
    BRIGHTNESS = 0,      // 1 byte: brightness (0-100)
    COLOR_RGB,          // 3 bytes: r, g, b (0-255 each)
    HEADING,           // 2 bytes: heading (int16_t)
    MAG_PID,          // 6 bytes: kp, ki, kd (int16_t each)
    SPEED,            // 2 bytes: speed (int16_t)
    MOVING,          // 3 bytes: angle (int16_t)
    STOP,           // 0 bytes
    EMERGENCY_STOP,// 0 bytes
    ACTIVATE,     // 0 bytes
    MAG_CALIBRATION // 0 bytes
};

void processCommand(const uint8_t* data, size_t length) {
    if (length < 1) return;  // Need at least command byte
    
    COMMAND cmd = static_cast<COMMAND>(data[0]);
    
    switch (cmd) {
        case BRIGHTNESS:
            if (length >= 2) {
                uint8_t brightness = data[1];
                brightness = map(brightness, 0, 100, 0, 255);
                led.setBrightness(brightness);
                led.setColorRGB();
            }
            break;
            
        case COLOR_RGB:
            if (length >= 4) {
                uint8_t r = data[1];
                uint8_t g = data[2];
                uint8_t b = data[3];
                led.setColorRGB(r, g, b);
            }
            break;
            
        case HEADING:
            if (length >= 3) {
                int16_t heading = (data[1] << 8) | data[2];
                mag.setTargetHeading(heading);
            }
            break;
            
        case MAG_PID:
            if (length >= 7) {
                int16_t kp = (data[1] << 8) | data[2];
                int16_t ki = (data[3] << 8) | data[4];
                int16_t kd = (data[5] << 8) | data[6];
                mag.setPIDTunings(kp / 1000.0, ki / 1000.0, kd / 1000.0);
            }
            break;
            
        case SPEED:
            if (length >= 3) {
                int16_t speed = (data[1] << 8) | data[2];
                mecanum.setSpeed(speed);
            }
            break;
            
        case MOVING:
            if (length >= 4 && calibrateTaskHandle == NULL) {
                int16_t angle = (data[1] << 8) | data[2];
                int8_t turnRate = static_cast<int8_t>(data[3]);
                int speed = mecanum.getSpeed();
                if (angle == 365){
                    mecanum.move(0, 0, speed*turnRate);
                }
                else {mecanum.move(angle, speed, speed*turnRate);}
                /*if (moveYawCompensatedTaskHandle != NULL) {
                    vTaskDelete(moveYawCompensatedTaskHandle);
                    moveYawCompensatedTaskHandle = NULL;
                }
                xTaskCreatePinnedToCore(moveYawCompensatedTask, "MoveYawCompensatedTask", 2048, &params, 1, &moveYawCompensatedTaskHandle, 1);*/
            }
            break;
            
        case STOP:
            if (moveYawCompensatedTaskHandle != NULL) {
                vTaskDelete(moveYawCompensatedTaskHandle);
                moveYawCompensatedTaskHandle = NULL;
            }
            mecanum.move(0, 0, 0);
            break;
            
        case EMERGENCY_STOP:
            if (moveYawCompensatedTaskHandle != NULL) {
                vTaskDelete(moveYawCompensatedTaskHandle);
                moveYawCompensatedTaskHandle = NULL;
            }
            mecanum.move(0, 0, 0);
            emergency.stop();
            break;
            
        case ACTIVATE:
            emergency.activate();
            break;
            
        case MAG_CALIBRATION:
            if (calibrateTaskHandle == NULL) {
                xTaskCreatePinnedToCore(calibrateTask, "CalibrateTask", 
                                      2048, NULL, 1, &calibrateTaskHandle, 1);
            }
            break;
            
        default:
            DEBUG_PRINTLN("Unknown command: " + String(cmd));
            return;
    }
    
    DEBUG_PRINTLN("Command executed: " + String(cmd) + " (length: " + String(length) + ")");
}

void blinkTask(void *pvParameters) {
    while (!ble.isConnected()) {
        led.setBrightness(50);
        led.blink();
    }
    led.setBrightness(10);
    led.setColorRGB(0, 255, 0);
    emergency.activate();
    DEBUG_PRINTLN("Connected to BLE device.");
    blinkTaskHandle = NULL;
    vTaskDelete(NULL);
}

void calibrateTask(void *pvParameters) {
    DEBUG_PRINTLN("Calibrating Magnetometer...");
    mecanum.rotate(100);
    if (mag.calibrate()) {
        mecanum.move(0,0,0);
        DEBUG_PRINTLN("Calibration successful.");
    } else {
        DEBUG_PRINTLN("Calibration failed.");
    }
    mecanum.move(0,0,0);
    calibrateTaskHandle = NULL;
    vTaskDelete(NULL);
}

void moveYawCompensatedTask(void *pvParameters) {
    int angle = *(int*)pvParameters;
    mag.setTargetHeading(mag.getHeading());
    while (true) {
        float heading = mag.getHeading();
        float turn = mag.computePID(heading);
        mecanum.move(angle, mecanum.getSpeed(), turn);
        //DEBUG_PRINTLN("Angle: " + String(angle) + " Turn: " + String(turn));
        vTaskDelay(100);
    }
}

void setup(){

    // Serial setup
    USBSerial.begin();
    USB.begin();

    // SkyRocket setup
    mecanum.begin();
    emergency.begin();
    led.init();


    //Magnetometer setup
    mag.begin(Wire);
    mag.initialize();        
    mag.setPIDTunings(1.0, 0.5, 0); // kp, ki, kd
    mag.setTargetHeading(90);

    // BLE setup
    ble.init();
    ble.setCommandCallback(processCommand);

    xTaskCreatePinnedToCore(blinkTask, "MyTask", 2048, NULL, 1, &blinkTaskHandle, 1);
}

void loop(){


    if (blinkTaskHandle == NULL && !ble.isConnected()) {
        DEBUG_PRINTLN("Disconnected from BLE device.");
        emergency.stop();
        xTaskCreatePinnedToCore(blinkTask, "MyTask", 2048, NULL, 1, &blinkTaskHandle, 1);
    }


}