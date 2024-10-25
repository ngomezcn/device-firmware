#include <Arduino.h>

class RelayController
{
private:
  int pinRelay;
  int pinIndicatorLed;
  unsigned long activationDuration;
  unsigned long relayStartTime;
  bool relayActivated;

public:
  // Constructor
  RelayController(int relayPin, int ledPin, unsigned long duration)
  {
    pinRelay = relayPin;
    pinIndicatorLed = ledPin;
    activationDuration = duration;
    relayActivated = false;
    
  }

  // Activar el relé de manera no bloqueante
  void activate()
  {
    digitalWrite(pinRelay, HIGH);
    digitalWrite(pinIndicatorLed, HIGH);
    relayStartTime = millis();
    relayActivated = true;
    Serial.println("Relé activado");
  }

  // Llamar en cada ciclo para verificar si debe apagarse el relé
  void update()
  {
    if (relayActivated && (millis() - relayStartTime >= activationDuration))
    {
      // Apagar el relé
      digitalWrite(pinRelay, LOW);
      digitalWrite(pinIndicatorLed, LOW);
      relayActivated = false;
      Serial.println("Relé desactivado");
    }
  }
};