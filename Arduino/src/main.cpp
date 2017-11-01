#include <Arduino.h>
#include <Light_Controller.h>

#define BUTTON_PIN 8
Light_Controller lightController;

void setup() {
  Serial.begin(9600);  // start serial for output
  pinMode(BUTTON_PIN, INPUT);
  lightController.calibrate();
}

// Loop variables
unsigned long startTime, endTime;

int in, out;
int incoming;
int state = 0;
int newState, oldState;

void loop() {
  startTime = millis();
  lightController.process();

  newState = digitalRead(BUTTON_PIN);
  if(newState != oldState && oldState){
    state = state ? 0 : 1;
    state ? lightController.setHighRef() : lightController.setLowRef();
  }
  oldState = newState;

  if (Serial.available() > 0) {
    // read the incoming byte:
    incoming = Serial.read();
    switch(incoming){
      case 49:
        lightController.setLowRef();
        state = 0;
        break;
      case 50:
        lightController.setHighRef();
        state = 1;
        break;
    }
  }

  endTime = millis();
  delay(lightController.getPeriod() - (endTime - startTime));
}
