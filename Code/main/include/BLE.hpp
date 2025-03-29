/*
 __ _            __            _        _   
/ _\ | ___   _  /__\ ___   ___| | _____| |_ 
\ \| |/ / | | |/ \/// _ \ / __| |/ / _ \ __|
_\ \   <| |_| / _  \ (_) | (__|   <  __/ |_ 
\__/_|\_\\__, \/ \_/\___/ \___|_|\_\___|\__|
         |___/                                                            
*/

/*  BLE.hpp    */

#ifndef BLE_HPP
#define BLE_HPP

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

class BLE {
public:
    BLE();
    ~BLE();

    void init();

    bool isConnected() const { return isConnected_; }

    using CommandCallback = void (*)(const String& command);
    void setCommandCallback(CommandCallback callback);

private:
    class MyServerCallbacks : public BLEServerCallbacks {
    public:
        MyServerCallbacks(BLE* ble) : ble_(ble) {}
        void onConnect(BLEServer* pServer) override;
        void onDisconnect(BLEServer* pServer) override;
    private:
        BLE* ble_;
    };

    class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
    public:
        MyCharacteristicCallbacks(BLE* ble) : ble_(ble) {}
        void onWrite(BLECharacteristic* pCharacteristic) override;
    private:
        BLE* ble_;
    };

    BLEServer* pServer_;
    BLECharacteristic* pCharacteristic_;
    bool isConnected_ = false;
    CommandCallback commandCallback_;
};

#endif