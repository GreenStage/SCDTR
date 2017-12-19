#ifndef LIGHT_H
#define LIGHT_H

#include "Arduino.h"
#include "TimerOne.h"
#include "pid_controller.h"

#define LDR_PIN A0
#define LED_PIN 9

#define R0 10000
#define VCC 1023

#define LDR_SL -0.691
#define LDR_OA 4.775

class light_controller {
  private:
    float _min_lux, _max_lux;
    float _r, _aw, _Kff;
    int _T, _u, _n = 3;
    int _ocupancy = 0;
    int _ff_mode = 1;
    pid_controller _pid;

    float _getVolt();
    float _volt2ohm(float v_in);
    float _volt2lux(float v_in);

  public:
    light_controller(int T, int r, float Kp, float Ki, float Kaw);

    void calibrate();
    void process();

    void setLight(int dc);
    void setMaxRef();
    void setMinRef();
    void setRef(float r);
    int getOcupancy();
    float getMaxRef();
    float getMinRef();
    float getRef();

    float getErr();
    float getLight();
    float getLowerIlluminance();
    float getIlluminance();
    int getDutyCycle();

    void initInterrupt();
    void startInterrupt();
    void stopInterrupt();
};

#endif
