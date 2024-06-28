#include<BLEDevice.h>
#include<BLEUtils.h>
#include<BLEServer.h>
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
BLECharacteristic *pCharacteristic;
void BLEinit(){
    Serial.begin(9600);
    BLEDevice::init("ESP32_BLE");
    BLEServer *pServer =BLEServer::createService();
    BLEServer *pService =pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID,BLECharacteristic::PROPERTY_READ |BLECharacteristic::PROPERTY_NOTIFY);
    pCharacteristic->setValue("Hello World");
    pService->start();
    pServer->getAdvertising()->start();
}