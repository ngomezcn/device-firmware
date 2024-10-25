#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "constants.h"       // PROD ONLY
#include "compile_defines.h" // PROD ONLY

#include "HeartBeat.cpp"
#include "RelayController.cpp"

extern Heartbeat heartbeat;
extern RelayController relayController;

BLECharacteristic *pCharacteristic;
BLEAdvertising *pAdvertising;
bool keyValid = false;

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    pCharacteristic->setValue("");

    digitalWrite(PIN_BLE_CONNECTION_LED, HIGH);

    pAdvertising->start();
    delay(500);
  };

  void onDisconnect(BLEServer *pServer)
  {
    digitalWrite(PIN_BLE_CONNECTION_LED, LOW);

    pAdvertising->start();
    delay(500);
  }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0)
    {
      std::string xd = std::string(AK_COMMAND_HEADER) + std::string(ACCESS_KEY);
      Serial.print(xd.length());
      Serial.println(xd.c_str());
      Serial.print(rxValue.length());
      Serial.println(rxValue.c_str());

      if ((rxValue.find(xd) != std::string::npos))
      {
        keyValid = true;
        Serial.println("OK");
        relayController.activate();

        pCharacteristic->setValue("OK");
      }
      else
      {
        keyValid = false;
        Serial.println("ERROR");
        pCharacteristic->setValue("ERROR");
      }
    }
  }
};

void setupSanityCheck()
{
  digitalWrite(PIN_HEARTBEAT_LED, HIGH);
  digitalWrite(PIN_RELAY_INDICATOR_LED, HIGH);
  digitalWrite(PIN_BLE_CONNECTION_LED, HIGH);

  delay(1500);

  digitalWrite(PIN_HEARTBEAT_LED, LOW);
  digitalWrite(PIN_RELAY_INDICATOR_LED, LOW);
  digitalWrite(PIN_BLE_CONNECTION_LED, LOW);
}

Heartbeat heartbeat(PIN_HEARTBEAT_LED, HEARTBEAT_ON_TIME_MS, HEARTBEAT_OFF_TIME_MS);
RelayController relayController(PIN_RELAY, PIN_RELAY_INDICATOR_LED, RELAY_ACTIVATION_DURATION);

void setup()
{

  pinMode(PIN_HEARTBEAT_LED, OUTPUT);
  digitalWrite(PIN_HEARTBEAT_LED, LOW);

  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);

  pinMode(PIN_RELAY_INDICATOR_LED, OUTPUT);
  digitalWrite(PIN_RELAY_INDICATOR_LED, LOW);

  pinMode(PIN_BLE_CONNECTION_LED, OUTPUT);
  digitalWrite(PIN_BLE_CONNECTION_LED, LOW);

  setupSanityCheck();

  Serial.begin(9600);
  Serial.println("iniciando");

  // Iniciar BLE
  BLEDevice::init(DEVICE_NAME);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Crear servicio
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Crear característica
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->setValue("Waiting for Key"); // Valor inicial

  // Añadir descriptor para notificaciones
  pCharacteristic->addDescriptor(new BLE2902());

  // Iniciar servicio
  pService->start();

  // Empezar a hacer publicidad del servicio
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();
}

void loop()
{
  heartbeat.update();
  relayController.update();

  delay(100);
}

/*#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

// #include "constants.h" // PROD ONLY
// #include "compile_defines.h" // PROD ONLY

//////////////////////////////
#define ACCESS_KEY "4321"
#define DEVICE_NAME "xddd"
#define RELAY_ACTIVATION_DURATION 5000

#define SERVICE_UUID "4c491e6a-38df-4d0f-b04b-8704a40071ce"
#define CHARACTERISTIC_UUID "b0726341-e52e-471b-9cd6-4061e54616cc"

#define PIN_RELAY 5
#define PIN_RELAY_INDICATOR_LED 2 ///  #define PIN_RELAY_INDICATOR_LED 32
#define PIN_BLE_CONNECTION_LED 26
#define PIN_HEARTBEAT_LED 2

#define HEARTBEAT_ON_TIME_MS 500
#define HEARTBEAT_OFF_TIME_MS 4000
///////////////////////////////

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
    digitalWrite(PIN_BLE_CONNECTION_LED, HIGH);
  }

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
    pAdvertising->start();
    digitalWrite(PIN_BLE_CONNECTION_LED, LOW);
  }
};

void sendOK()
{
  if (deviceConnected)
  {
    //pCharacteristic->setValue("WHAT");
    //pCharacteristic->notify();
    //Serial.println("OK");
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
      BLECharacteristic::PROPERTY_READ  |
      BLECharacteristic::PROPERTY_WRITE |
      BLECharacteristic::PROPERTY_NOTIFY
    ); // Agregar propiedad de notificación

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

      //if (value.startsWith("AK=") && value.substring(3) == ACCESS_KEY)
      if (true)
      {
            pCharacteristic->setValue("WHAT");

        pCharacteristic->notify();
        Serial.println("OK");

        sendOK();
      }

    }

  }

  delay(10);
}*/