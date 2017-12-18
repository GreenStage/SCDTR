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
    double _min_lux, _max_lux;
    double _u, _aw, _Kff;
    int _T, _r, _o, _n = 3;
    int _ff_mode = 1;
    int _K[];
    pid_controller _pid;

    double _getVolt();
    double _volt2ohm(double v_in);
    double _volt2lux(double v_in);

  public:
    light_controller(int T, int r, double Kp, double Ki, double Kaw);

    void calibrate();
    void process();

    void setLight(int dc);
    void setMaxRef();
    void setMinRef();

    double getLight();
    double getIlluminance();

    void initInterrupt();
    void startInterrupt();
    void stopInterrupt();
};

#endif
