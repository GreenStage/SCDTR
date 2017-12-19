#include "pid_controller.h"

pid_controller::pid_controller(int T, int r, double Kp, double Ki, double Kaw) {
  _T = T;
  _r = r;
  _Kp = Kp;
  _Ki = Kp * Ki * _T/2;
  _Kaw = Kaw;
}

void pid_controller::_deadzone() {
  if(_e > 10) _e -= 10;
  else if(_e < -10) _e += 10;
}

double pid_controller::process() {
  _e = _r - _y;
  _deadzone();
  _p = _Kp * _e;
  _i = _i_prev + _Ki * (_e + _e_prev);
  return _p + _i;
}

int pid_controller::saturate(double aw){
  if(aw > 255){
     _i -= _Kaw*(aw - 255);
    return 255;
  } else if(aw < 0){
     _i -= _Kaw * aw;
     return 0;
  }
  return round(aw);
}

void pid_controller::flush(){
  _e_prev = _e;
  _i_prev = _i;
}

int pid_controller::getRef(){ return _r; }
void pid_controller::setRef(int r){ _r = r; }
double pid_controller::getErr(){ return _e; }
double pid_controller::getLight(){ return _y; }
void pid_controller::setLight(double y){ _y = y; }
