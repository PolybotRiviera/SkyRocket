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
#include <Wire.h>
#include "include/Mecanum.hpp"
#include "include/Emergency.hpp"
#include "include/LED.hpp"
#include "include/BLE.hpp"
#include "include/Magnetometer.hpp"
#include "include/lidar.hpp"
#include "include/Hedgehog.hpp"
#include "USB.h"

#define DEBUG true
#define DEBUG_PRINTLN(x) if (DEBUG) USBSerial.println(x)

USBCDC USBSerial;
Mecanum mecanum;
Emergency emergency;
Hedgehog hedgehog(Serial, 115200);
LED led(21, 1, NEO_GRB + NEO_KHZ800);
BLE ble;
Magnetometer mag;
HardwareSerial SerialLidar(1);
Lidar lidar(SerialLidar, 18);

TaskHandle_t blinkTaskHandle; 
TaskHandle_t calibrateMagTaskHandle;
TaskHandle_t calibrateBeaconTaskHandle;
TaskHandle_t yawCompensatedTaskHandle;
TaskHandle_t moveTaskHandle;

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
    MAG_CALIBRATION, // 0 bytes
    YAW_COMPENSATED_TOGGLE, // 0 bytes
    CALIBRATE_BEACON, // 0 bytes
    GOTO, // 4 bytes: x, y (16 bit int each)
    BEACON_PID, // 6 bytes: kp, ki, kd (int16_t each)
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
            if (length >= 4 && calibrateMagTaskHandle == NULL) {
                int16_t angle = (data[1] << 8) | data[2];
                int8_t turnRate = static_cast<int8_t>(data[3]);
                mecanum.setAngle(angle);
                mecanum.setTurn(turnRate * mecanum.getSpeed());
                mecanum.setState(1);
            }
            break;
            
        case STOP:
            mecanum.setTurn(0);
            mecanum.setAngle(0);
            mecanum.setState(0);
            if (moveTaskHandle != NULL) {
                vTaskDelete(moveTaskHandle);
                moveTaskHandle = NULL;
            }
            if (calibrateMagTaskHandle != NULL) {
                vTaskDelete(calibrateMagTaskHandle);
                calibrateMagTaskHandle = NULL;
            }
            if (calibrateBeaconTaskHandle != NULL) {
                vTaskDelete(calibrateBeaconTaskHandle);
                calibrateBeaconTaskHandle = NULL;
            }
            break;
            
        case EMERGENCY_STOP:
            if (moveTaskHandle != NULL) {
                vTaskDelete(moveTaskHandle);
                moveTaskHandle = NULL;
            }
            if (calibrateMagTaskHandle != NULL) {
                vTaskDelete(calibrateMagTaskHandle);
                calibrateMagTaskHandle = NULL;
            }
            emergency.stop();
            break;
            
        case ACTIVATE:

            if (moveTaskHandle == NULL) {
                xTaskCreatePinnedToCore(moveTask, "MoveTask", 2048, NULL, 1, &moveTaskHandle, 1);
            }
            emergency.activate();
            break;
            
        case MAG_CALIBRATION:
            mecanum.setState(0);
            if (calibrateMagTaskHandle == NULL) {
                xTaskCreatePinnedToCore(calibrateMagTask, "calibrateMagTask", 2048, NULL, 1, &calibrateMagTaskHandle, 1);
            }
            break;

        case YAW_COMPENSATED_TOGGLE:
            if (yawCompensatedTaskHandle != NULL) {
                vTaskDelete(yawCompensatedTaskHandle);
                yawCompensatedTaskHandle = NULL;
                mag.setCorrection(0);
            } 
            else {
                xTaskCreatePinnedToCore(yawCompensatedTask, "MoveYawCompensatedTask", 2048, NULL, 1, &yawCompensatedTaskHandle, 1);
            }
            break;

        case CALIBRATE_BEACON:
            if (calibrateBeaconTaskHandle == NULL) {
                xTaskCreatePinnedToCore(calibrateBeaconTask, "CalibrateBeaconTask", 2048, NULL, 1, &calibrateBeaconTaskHandle, 1);
            }
            break;
        
        case GOTO:
            if (length >= 5) {
                int16_t x = (data[1] << 8) | data[2];
                int16_t y = (data[3] << 8) | data[4];
                hedgehog.setTarget(x, y);
                if (moveTaskHandle != NULL) {
                    vTaskDelete(moveTaskHandle);
                    moveTaskHandle = NULL;
                }
                xTaskCreatePinnedToCore(goToTask, "GoToTask", 2048, NULL, 1, &moveTaskHandle, 1);
            }
            break;

        case BEACON_PID:
            if (length >= 7) {
                int16_t kp = (data[1] << 8) | data[2];
                int16_t ki = (data[3] << 8) | data[4];
                int16_t kd = (data[5] << 8) | data[6];
                hedgehog.setPIDTunings(kp / 1000.0, ki / 1000.0, kd / 1000.0);
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

void calibrateMagTask(void *pvParameters) {
    DEBUG_PRINTLN("Calibrating Magnetometer...");
    mecanum.setTurn(-100);
    if (mag.calibrate()) {
        DEBUG_PRINTLN("Calibration successful.");
    } else {
        DEBUG_PRINTLN("Calibration failed.");
    }
    mecanum.setTurn(0);
    calibrateMagTaskHandle = NULL;
    vTaskDelete(NULL);
}

void yawCompensatedTask(void *pvParameters) {
    while (true) {
        float heading = mag.getHeading();
        float turn = mag.computePID(heading);
        mag.setCorrection(turn);
        vTaskDelay(10);
    }
    vTaskDelete(NULL);
}

void calibrateBeaconTask(void *pvParameters) {
    DEBUG_PRINTLN("Calibrating Beacon...");
    float heading = mag.getHeading();
    float x1 = hedgehog.getX();
    float y1 = hedgehog.getY();
    mecanum.move(0,30,0);
    vTaskDelay(2000);
    float x2 = hedgehog.getX();
    float y2 = hedgehog.getY();
    mecanum.move(0,0,0);
    float distance = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
    float angle = atan2(y2 - y1, x2 - x1) * 180 / PI;
    if (angle < 0) {
        angle += 360;
    }
    hedgehog.setAngle(angle - mag.getHeading());
    DEBUG_PRINTLN("Beacon calibration complete. Distance: " + String(distance) + ", Angle: " + String(angle-mag.getHeading()));
    vTaskDelete(NULL);
}

void moveTask(void *pvParameters) {
    while (true) {
        mecanum.move(mecanum.getAngle(), mecanum.getSpeed() * mecanum.getState(), mecanum.getTurn() + mag.getCorrection());
        vTaskDelay(10);
    }
    vTaskDelete(NULL);
}

void goToTask(void *pvParameters) {

    hedgehog.update();
    int targetX = hedgehog.getTargetX();
    int targetY = hedgehog.getTargetY();
    targetX = 1000;
    targetY = 1000;
    float distanceToTarget = sqrt(pow(targetX - hedgehog.getX(), 2) + pow(targetY - hedgehog.getY(), 2));
    float threshold = hedgehog.getThreshold();

    while (distanceToTarget > threshold) {
        hedgehog.update();
        float heading = mag.getHeading();
        float x = hedgehog.getX();
        float y = hedgehog.getY();
        float angleToTarget = atan2(targetY - y, targetX - x) * 180 / PI;
        if (angleToTarget < 0) {
            angleToTarget += 360;
        }
        distanceToTarget = sqrt(pow(targetX - x, 2) + pow(targetY - y, 2));
        int speed = hedgehog.computePID(distanceToTarget);
        mecanum.move(angleToTarget, speed, mecanum.getTurn() + mag.getCorrection());
        vTaskDelay(10);
    }
    vTaskDelete(NULL);
}

void setup(){

    // Serial setup
    USBSerial.begin();
    USB.begin();

    // SkyRocket setup
    mecanum.begin();
    emergency.begin();
    led.init();
    lidar.init();
    hedgehog.init();


    //Magnetometer setup
    mag.begin(Wire);
    mag.initialize();        
    mag.setPIDTunings(3.0, 0, 0); // kp, ki, kd
    mag.setTargetHeading(90);

    // BLE setup
    ble.init();
    ble.setCommandCallback(processCommand);

    //xTaskCreatePinnedToCore(blinkTask, "BlinkTask", 2048, NULL, 1, &blinkTaskHandle, 1);

    if (moveTaskHandle != NULL) {
        vTaskDelete(moveTaskHandle);
        moveTaskHandle = NULL;
    }

    //xTaskCreatePinnedToCore(moveTask, "MoveTask", 2048, NULL, 1, &moveTaskHandle, 1);
    //xTaskCreatePinnedToCore(goToTask, "MoveTask", 2048, NULL, 1, &moveTaskHandle, 1);
    //xTaskCreatePinnedToCore(calibrateBeaconTask, "calibrateBeaconTask", 2048, NULL, 1, &calibrateBeaconTaskHandle, 1);
}

void loop(){
    
    if (blinkTaskHandle == NULL && !ble.isConnected()) {
        DEBUG_PRINTLN("Disconnected from BLE device.");
        emergency.stop();
        xTaskCreatePinnedToCore(blinkTask, "BlinkTask", 2048, NULL, 1, &blinkTaskHandle, 1);
    }
    vTaskDelay(10);

}