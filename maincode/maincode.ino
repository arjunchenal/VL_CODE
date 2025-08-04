#include <Arduino.h>
#include "VarLogger.h"

const int ledPin = 13;

int temperature = 25;
int humidity = 50;
int pressure = 1000;
int counter = 0;
int specialEvent = 0;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  VarLogger::init();

  if (!VarLogger::sd_initialized(10)) {
    Serial.println("SD initialization failed");
    while (1)
      ;
  }
}

void loop() {
  temperature = 20 + random(-3, 4);   // temp
  humidity = 40 + random(-5, 5);      // humidity
  pressure = 1000 + random(-10, 10);  // pressure
  counter++;
  specialEvent += random(0, 3);

  digitalWrite(ledPin, (counter % 2) ? HIGH : LOW);

  VarLogger::log("temperature", "loop", "sensor", "thread1", temperature, true);
  VarLogger::log("humidity", "loop", "sensor", "thread1", humidity, true);
  VarLogger::log("pressure", "loop", "sensor", "thread1", pressure, true);

  VarLogger::log("counter", "loop", "system", "thread2", counter, true);
  VarLogger::log("specialEvent", "loop", "system", "thread2", specialEvent, true);

  VarLogger::log("randomValue", "loop", "simulator", "thread3", random(0, 100), true);

  if (counter % 15 == 0) {
    VarLogger::log("milestoneEvent", "loop", "system", "thread4", random(1000, 2000), true);
    Serial.println("[INFO] Milestone event logged");
  }

  if (counter == 200) {
    Serial.println("\nTesting complete. Flushing any remaining data");
    VarLogger::flush();
    Serial.println("Data flushed. Stop");
    while (1)
      ;
  }

  delay(300);
}
