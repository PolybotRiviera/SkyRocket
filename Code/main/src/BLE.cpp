/*
 __ _            __            _        _   
/ _\ | ___   _  /__\ ___   ___| | _____| |_ 
\ \| |/ / | | |/ \/// _ \ / __| |/ / _ \ __|
_\ \   <| |_| / _  \ (_) | (__|   <  __/ |_ 
\__/_|\_\\__, \/ \_/\___/ \___|_|\_\___|\__|
         |___/                                                            
*/

/*  BLE.cpp    */

#include <Arduino.h>
#include "../include/BLE.hpp"

BLE::BLE()
    : pServer_(nullptr),
      pCharacteristic_(nullptr),
      isConnected_(false),
      commandCallback_(nullptr) {
}

BLE::~BLE() {
    if (pServer_) {
        pServer_->getAdvertising()->stop();
        delete pServer_;
    }
}

void BLE::init() {
    BLEDevice::init("SkyRocket");
    pServer_ = BLEDevice::createServer();
    pServer_->setCallbacks(new MyServerCallbacks(this));

    BLEService *pService = pServer_->createService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
    pCharacteristic_ = pService->createCharacteristic(
        "beb5483e-36e1-4688-b7f5-ea07361b26a8",
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
    );

    pCharacteristic_->setCallbacks(new MyCharacteristicCallbacks(this));
    
    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
    pAdvertising->setScanResponse(true);
    pAdvertising->start();
}

void BLE::setCommandCallback(CommandCallback callback) {
    commandCallback_ = callback;
}

void BLE::MyServerCallbacks::onConnect(BLEServer* pServer) {
    ble_->isConnected_ = true;
}

void BLE::MyServerCallbacks::onDisconnect(BLEServer* pServer) {
    ble_->isConnected_ = false;
    pServer->startAdvertising();
}

void BLE::MyCharacteristicCallbacks::onWrite(BLECharacteristic* pCharacteristic) {
    if (ble_->isConnected_ && ble_->commandCallback_) {
        std::string value = pCharacteristic->getValue();
        ble_->commandCallback_(reinterpret_cast<const uint8_t*>(value.data()), value.length());
        pCharacteristic->setValue("");
    }
}