#include <Arduino.h>
#include <Light_Controller.h>

Light_Controller lightController;

void setup() {
  Serial.begin(9600);  // start serial for output
  lightController.calibrate();
  lightController.setHighRef();

}

// Loop variables
unsigned long startTime, endTime;

int in, out;
int incoming;

void loop() {
  /*
  startTime = millis();
  //lightController.process();

  if (Serial.available() > 0) {
    // read the incoming byte:
    incoming = Serial.read();
    switch(incoming){
      case 49:
        lightController.setLowRef();
        break;
      case 50:
        lightController.setHighRef();
        break;
      case 51:
        lightController.toggleFeedForward();
        break;
      case 52:
        lightController.calibrate();
        break;
    }
  }

  endTime = millis();
  delay(lightController.getPeriod() - (endTime - startTime));
  */
}
