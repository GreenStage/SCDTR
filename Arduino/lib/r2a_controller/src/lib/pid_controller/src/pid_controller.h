#ifndef PID_H
#define PID_H

#include "Arduino.h"

class pid_controller {
  private:
    int _T, _r, _u;
    float _Kp, _Ki, _Kaw;
    float _y, _e, _p, _i, _aw;
    float _y_prev, _e_prev, _i_prev;

    void _deadzone();
    
  public:
    pid_controller(int T, int r, float Kp, float Ki, float Kaw);

    void flush();
    float process();
    int saturate(float y);

    int getRef();
    float getErr();
    float getLight();

    void setRef(int r);
    void setLight(float y);
};

#endif
