#include <Arduino.h>
#include "light_controller.h"
#include "i2c_controller.h"
#include "network_controller.h"

extern light_controller lc;
extern i2c_controller i2c;
extern network_controller nc;

void setup() {
    Serial.begin(9600);
    nc.init();
    nc.calibrate();
    nc.consensus();
    //lc.calibrate();
    lc.initInterrupt();
}

void loop()
{
  nc.process();
}
