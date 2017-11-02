#ifndef LIGHT_H
#define LIGHT_H

#include "PID_Controller.h"

#define LDR_PIN A0
#define LED_PIN 9

#define R0 10000
#define VCC 1023

#define LDR_SL -0.691
#define LDR_OA 4.775

class Light_Controller {
  private:
    double _min_lux, _max_lux;
    int _n;

    float volt2ohm(int v_in);
    float volt2lux(int v_in);

  public:
    // Getters
    int getLight();
    int getPeriod();

    // Setters
    void setLowRef();
    void setHighRef();

    // Commands
    void process();
    void calibrate();
};

#endif
