#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "constants.h"
#include "compile_defines.h" // PROD ONLY



// Variables globales
BLEServer *pServer = nullptr;
BLECharacteristic *pCharacteristic = nullptr;
BLEAdvertising *pAdvertising = nullptr; // Mover la declaración aquí
bool deviceConnected = false;

// Declarar la función sendLEDState antes de la clase
void sendOK();

// Clase para manejar conexiones del cliente
class MyServerCallbacks : public BLEServerCallbacks
{
public:
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
    pAdvertising->start();
  }

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
    pAdvertising->start();
  }
};

void sendOK()
{
  if (deviceConnected)
  {
    pCharacteristic->setValue("OK");
    pCharacteristic->notify();
    Serial.print("OK");
  }
}

void setup()
{
  Serial.begin(9600);

  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_RELAY_INDICATOR_LED, OUTPUT);
  pinMode(PIN_BLE_CONNECTION_LED, OUTPUT);
  pinMode(PIN_HEARTBEAT_LED, OUTPUT);

  digitalWrite(PIN_RELAY, LOW);
  digitalWrite(PIN_RELAY_INDICATOR_LED, LOW);
  digitalWrite(PIN_BLE_CONNECTION_LED, LOW);
  digitalWrite(PIN_HEARTBEAT_LED, LOW);

  // Inicializar BLE
  BLEDevice::init(DEVICE_NAME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Crear un servicio BLE
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Crear una característica BLE
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY); // Agregar propiedad de notificación

  // Iniciar el servicio
  pService->start();

  // Inicializar la publicidad BLE
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinInterval(100);
  pAdvertising->setMaxInterval(200);
  pAdvertising->setAdvertisementType(ADV_TYPE_IND);

  // Iniciar el advertising
  pAdvertising->start();

  Serial.println("Advertising started...");
}

void loop()
{
  if (deviceConnected)
  {
    if (pCharacteristic->getValue().length() > 0)
    {
      String value = pCharacteristic->getValue().c_str();
      Serial.print("Valor recibido: ");
      Serial.println(value);

      if (value.indexOf("AK=" + String(ACCESS_KEY)) >= 0)
      {
        sendOK();
      }

      pCharacteristic->setValue("");
    }
  }

  delay(10);
}