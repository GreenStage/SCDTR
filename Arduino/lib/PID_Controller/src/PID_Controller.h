#ifndef PID_H
#define PID_H

#include "Arduino.h"

class pid_controller {
  private:
    int _T, _r;
    double _Kp, _Ki, _Kaw;
    double _y, _e, _p, _i, _aw, _u;
    double _y_prev, _e_prev, _i_prev;

    void _deadzone();
  public:
    // Constructor
    pid_controller(int T, int r, double Kp, double Ki, double Kaw);

    double process();
    int saturate(double aw);
    void flush();

    void setRef(int r);
    int getRef();
    void setLight(double y);
    double getLight();
};

#endif
