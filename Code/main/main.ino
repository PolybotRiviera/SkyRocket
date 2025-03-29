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
#include "include/Mecanum.hpp"
#include "include/Emergency.hpp"
#include "include/LED.hpp"
#include "include/BLE.hpp"
#include "include/Magnetometer.hpp"
#include "USB.h"

#include <unordered_map>

USBCDC USBSerial;
Mecanum mecanum;
Emergency emergency;
LED led(21, 1, NEO_GRB + NEO_KHZ800);
BLE ble;
Magnetometer mag;

TaskHandle_t blinkTaskHandle; 
TaskHandle_t calibrateTaskHandle;
TaskHandle_t moveYawCompensatedTaskHandle;

void processCommand(const String& command) {
    static const std::unordered_map<String, void(*)(const String&)> commandHandlers = {
        {"Brightness", [](const String& cmd) {
            int brightness;
            if (sscanf(cmd.c_str(), "Brightness %d", &brightness) == 1) {
                brightness = map(brightness, 0, 100, 0, 255);
                led.setBrightness(brightness);
                led.setColorRGB();
            }
        }},
        {"ColorRGB", [](const String& cmd) {
            int r, g, b;
            if (sscanf(cmd.c_str(), "ColorRGB %d %d %d", &r, &g, &b) == 3) {
                led.setColorRGB(r, g, b);
            }
        }},
        {"Heading", [](const String& cmd) {
            int heading;
            if (sscanf(cmd.c_str(), "Heading %d", &heading) == 1) {
                mag.setTargetHeading(heading);
            }
        }},
        {"magPID", [](const String& cmd) {
            int kp, ki, kd;
            if (sscanf(cmd.c_str(), "magPID %d %d %d", &kp, &ki, &kd) == 3) {
                mag.setPIDTunings(kp, ki, kd);
            }
        }},
        {"Speed", [](const String& cmd) {
            int speed;
            if (sscanf(cmd.c_str(), "Speed %d", &speed) == 1) {
                mecanum.setSpeed(speed);
            }
        }},
        {"Moving", [](const String& cmd) {
            int angle;
            if (sscanf(cmd.c_str(), "Moving %d", &angle) == 1 && calibrateTaskHandle == NULL) {
                if (moveYawCompensatedTaskHandle != NULL) {
                    vTaskDelete(moveYawCompensatedTaskHandle);
                    moveYawCompensatedTaskHandle = NULL;
                }
                xTaskCreatePinnedToCore(moveYawCompensatedTask, "MoveYawCompensatedTask", 2048, &angle, 1, &moveYawCompensatedTaskHandle, 1);
            }
        }},
        {"stop", [](const String&) {
            if (moveYawCompensatedTaskHandle != NULL) {
                vTaskDelete(moveYawCompensatedTaskHandle);
                moveYawCompensatedTaskHandle = NULL;
            }
            mecanum.move(0, 0, 0);
        }},
        {"emergency_stop", [](const String&) {
            if (moveYawCompensatedTaskHandle != NULL) {
                vTaskDelete(moveYawCompensatedTaskHandle);
                moveYawCompensatedTaskHandle = NULL;
            }
            mecanum.move(0, 0, 0);
            emergency.stop();
        }},
        {"activate", [](const String&) {
            emergency.activate();
        }},
        {"magCalibration", [](const String&) {
            if (calibrateTaskHandle == NULL) {
                xTaskCreatePinnedToCore(calibrateTask, "CalibrateTask", 2048, NULL, 1, &calibrateTaskHandle, 1);
            }
        }}
    };

    for (const auto& pair : commandHandlers) {
        if (command.startsWith(pair.first)) {
            pair.second(command);
            USBSerial.print("Received command: ");
            USBSerial.println(command);
            return;
        }
    }

    USBSerial.print("Unknown command: ");
    USBSerial.println(command);
}

void blinkTask(void *pvParameters) {
    while (!ble.isConnected()) {
        led.setBrightness(50);
        led.blink();
    }
    led.setBrightness(10);
    led.setColorRGB(0, 255, 0);
    emergency.activate();
    blinkTaskHandle = NULL;
    vTaskDelete(NULL);
}

void calibrateTask(void *pvParameters) {
    USBSerial.println("Calibrating magnetometer...");
    mecanum.rotate(100);
    if (mag.calibrate()) {
        mecanum.move(0,0,0);
        USBSerial.println("Calibration complete.");
    } else {
        USBSerial.println("Calibration failed.");
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
        mecanum.move(angle, mag.getHeading(), turn);
        USBSerial.print("Angle: ");
        USBSerial.print(angle);
        USBSerial.print(" Turn: ");
        USBSerial.println(turn);
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

    //USBSerial.println(ble.isConnected());
    //delay(1000);

    // float heading = mag.getHeading();
    // USBSerial.print("Compass Heading: ");
    // USBSerial.println(heading);
    // delay(50);

    /*if (calibrateTaskHandle == NULL){
        float heading = mag.getHeading();
        float turn = mag.computePID(heading);
        mecanum.rotate(turn);
        vTaskDelay(50);
    }*/


    if (blinkTaskHandle == NULL && !ble.isConnected()) {
        emergency.stop();
        xTaskCreatePinnedToCore(blinkTask, "MyTask", 2048, NULL, 1, &blinkTaskHandle, 1);
    }

    //mecanum.move(0, 50, 0);

}