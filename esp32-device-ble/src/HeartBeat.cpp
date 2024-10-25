#include <Arduino.h>

class Heartbeat {
  private:
    int pin;
    unsigned long onTime;
    unsigned long offTime;
    unsigned long lastToggleTime;
    bool ledState;

  public:
    // Constructor de la clase Heartbeat
    Heartbeat(int ledPin, unsigned long onDuration, unsigned long offDuration) {
      pin = ledPin;
      onTime = onDuration;
      offTime = offDuration;
      lastToggleTime = 0;
      ledState = false;

    }

    // Método para actualizar el estado del LED
    void update() {
      unsigned long currentTime = millis();
      
      if (ledState && currentTime - lastToggleTime >= onTime) {
        // Si el LED está encendido y ya pasó el tiempo de encendido, apagarlo
        digitalWrite(pin, LOW);
        ledState = false;
        lastToggleTime = currentTime;
      } else if (!ledState && currentTime - lastToggleTime >= offTime) {
        // Si el LED está apagado y ya pasó el tiempo de apagado, encenderlo
        digitalWrite(pin, HIGH);
        ledState = true;
        lastToggleTime = currentTime;
      }
    }
};
