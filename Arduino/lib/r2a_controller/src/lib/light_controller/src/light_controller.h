#ifndef LIGHT_H
#define LIGHT_H

#include "Arduino.h"
#include "TimerOne.h"
#include "pid_controller.h"
#include "state_controller.h"

#define LDR_PIN A0
#define LED_PIN 9

#define R0 10000
#define VCC 1023

#define LDR_SL -0.691
#define LDR_OA 4.775

class light_controller {
  private:
    pid_controller _pid;
    state_controller &_state;

    float _getVolt();
    float _volt2ohm(float v);
    float _volt2lux(float v);

  public:
    light_controller(state_controller &state);

    void init(void (*cb)());
    void calibrate();
    void process();

    void setLight(int u);
    float getLight();

    void startInterrupt();
    void stopInterrupt();
    void initInterrupt();
};

#endif
