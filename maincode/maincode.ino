#include <Arduino.h>
#include "VarLogger.h"  // Include the VarLogger class

const int ledPin = 13; // Pin connected to the LED
int ledState = LOW;    // State of the LED

//for testing
int testVar1 = 0;
int testVar2 = 100;
int testVar3 = 200;


void setup() {
    Serial.begin(9600);
    pinMode(ledPin, OUTPUT);  // Set the pin mode for LED
    VarLogger::init();        // Initialize the logger
}

void loop() {
    testVar1++;
    testVar2--;
    testVar3--;
    // Toggle the LED state
    ledState = !ledState;
    digitalWrite(ledPin, ledState);

    // Log the LED state
    //VarLogger::log("LED", "loop", "main", "thread1", ledState);
    VarLogger::log("testVar1", "loop", "main", "thread1", testVar1, true);
    VarLogger::log("testVar2", "loop", "main", "thread1", testVar2, true);
    VarLogger::log("testVar3", "loop", "main", "thread1", testVar3, true); 

    int randomValue = random(0, 50);  //To test a random sensor reading
    VarLogger::log("randomValue", "loop", "main", "thread1", randomValue,true);


    if (testVar1 % 10 == 0) {
        VarLogger::save();
    }

    delay(1000);  // Wait for 1 second
}
