#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "access_key.h"
#include "device_name.h"
#include "relay_active_period.h"

// Define el UUID del servicio y la característica
#define SERVICE_UUID "4c491e6a-38df-4d0f-b04b-8704a40071ce"
#define CHARACTERISTIC_UUID "b0726341-e52e-471b-9cd6-4061e54616cc"

const int statusLedPin = 2;
const int gateRelayPin = 5;

// Variables para el parpadeo del LED de estado
unsigned long previousMillis = 0;
unsigned long relayActiveMillis = 0; // Para controlar el tiempo del relé
bool relayActive = false; // Estado del relé

// Variables globales
BLEServer *pServer = nullptr;
BLECharacteristic *pCharacteristic = nullptr;
BLEAdvertising *pAdvertising = nullptr; // Mover la declaración aquí
bool deviceConnected = false; 

// Declarar la función sendLEDState antes de la clase
void sendLEDState();

// Clase para manejar conexiones del cliente
class MyServerCallbacks : public BLEServerCallbacks
{
public:
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
    previousMillis = 0;   // Reiniciar el tiempo
    sendLEDState();       // Enviar el estado del LED al cliente al conectar

    // Reiniciar la publicidad
    pAdvertising->start(); // Reiniciar la publicidad al conectar
  }

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
    pAdvertising->start(); // Reiniciar la publicidad al desconectar
  }
};

// Función para enviar el estado del relé al cliente
void sendLEDState()
{
  if (deviceConnected)
  {
    String state = relayActive ? "RELAY ACTIVADO" : "RELAY DESACTIVADO";
    pCharacteristic->setValue(state.c_str()); // Configura el valor de la característica
    pCharacteristic->notify();                // Envía la notificación al cliente
    Serial.print("Estado del relé enviado: ");
    Serial.println(state);
  }
}

void setup()
{
  // Inicializar el puerto serie
  Serial.begin(9600);

  // Inicializar el pin del LED y del relé
  pinMode(statusLedPin, OUTPUT);
  pinMode(gateRelayPin, OUTPUT);

  digitalWrite(statusLedPin, LOW);
  digitalWrite(gateRelayPin, LOW);

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
  pAdvertising = BLEDevice::getAdvertising(); // Inicializa el objeto pAdvertising
  pAdvertising->addServiceUUID(SERVICE_UUID); // Añadir el UUID del servicio
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinInterval(100);
  pAdvertising->setMaxInterval(200);
  pAdvertising->setAdvertisementType(ADV_TYPE_IND);

  // Iniciar el advertising
  pAdvertising->start();

  // Mostrar la dirección MAC en el puerto serie
  Serial.print("\n\nDirección MAC: ");
  Serial.println(BLEDevice::getAddress().toString().c_str()); 

  Serial.println("Advertising started...");
}

void blinkStatusLedPeriodically()
{
  unsigned long currentMillis = millis();

  // Cambiar el estado del LED dependiendo de la conexión
  if (deviceConnected)
  {
    // Parpadeo rápido
    if (currentMillis - previousMillis >= 200)
    {
      previousMillis = currentMillis;
      digitalWrite(statusLedPin, !digitalRead(statusLedPin)); // Alternar el estado del LED
    }
  }
  else
  {
    // Si no está conectado, asegúrate de que el LED esté apagado
    digitalWrite(statusLedPin, LOW); // Apagar LED
  }
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

      // Comprobar si el valor recibido contiene el substring "AK=" seguido de la clave de acceso
      if (value.indexOf("AK=1234") >= 0) 
      {
        if (!relayActive) // Solo activar si el relé no está activo
        {
          digitalWrite(gateRelayPin, HIGH); // Activar el relé
          relayActive = true; // Marcar como activo
          relayActiveMillis = millis(); // Guardar el tiempo de activación
          Serial.println("Relé activado.");
          sendLEDState(); // Enviar el estado del relé al cliente
        }
      }

      // Limpiar el valor después de procesarlo
      pCharacteristic->setValue(""); // Reiniciar valor
    }
  }

  // Controlar el estado del relé
  if (relayActive && (millis() - relayActiveMillis >= RELAY_ACTIVATION_DURATION))
  {
    digitalWrite(gateRelayPin, LOW); // Desactivar el relé
    relayActive = false; // Marcar como inactivo
    Serial.println("Relé desactivado.");
    sendLEDState(); // Enviar el estado del relé al cliente
  }

  // Llamar a la función de parpadeo del LED
  blinkStatusLedPeriodically();

  // Evitar que el loop consuma mucha CPU
  delay(10); // Un pequeño delay para evitar un uso excesivo de la CPU
}