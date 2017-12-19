#ifndef PID_H
#define PID_H

#include "Arduino.h"

class pid_controller {
  private:
    int _T, _r, _u;
    double _Kp, _Ki, _Kaw;
    double _y, _e, _p, _i, _aw;
    double _y_prev, _e_prev, _i_prev;

    void _deadzone();
  public:
    // Constructor
    pid_controller(int T, int r, double Kp, double Ki, double Kaw);

    double process();
    int saturate(double aw);
    void flush();

    int getRef();
    void setRef(int r);
    double getErr();
    double getLight();
    void setLight(double y);
};

#endif
