#include "Arduino.h"
#include "PID_Controller.h"

PID_Controller::PID_Controller(int T, int r, double Kp, double Ki, double Kd, int a, int b) {
  _y_prev = _e_prev = _i_prev = _d_prev = 0;
  _Ts = T;
  _r = r;
  setGains(Kp, Ki, Kd, a, b);
}

int PID_Controller::getRef(){ return _r; }

void PID_Controller::setRef(int r){ _r = r; }
void PID_Controller::incRef(int v){ _r += v; }
void PID_Controller::decRef(int v){ _r -= v; }

void PID_Controller::setPeriod(int T){ _Ts = T; }

void PID_Controller::setFFGain(int Kff){ _Kff = Kff; }
void PID_Controller::setGains(double Kp, double Ki, double Kd, int a, int b){
  _K = Kp;
  _K1 = Kp * b;
  _K2 = Kp * Ki * _Ts/2;
  _K3 = Kd / (Kd + a * _Ts);
  _K4 = Kp * Kd * a / (Kd + a * _Ts);
}

void PID_Controller::sample(float y) { _y = y; }

void PID_Controller::_deadzone() {
  if(_e > 1) _e -= 1;
  else if(_e < -1) _e += 1;
}

void PID_Controller::_saturate() {
  if(_aw > 255){
     _u = 255;
     _i -= _aw - 255;
  } else if(_aw < 0){
     _u = 0;
     _i -= _aw;
  } else {
     _u = round(_aw);
  }
}

int PID_Controller::process(){
  _e = _r - _y;
  //Serial.println("Y: " + String(_y));
  _deadzone();

  //Serial.println("Y: " + String(_y));
  //Serial.println("E: " + String(_e));

  _p = _K1 * _r - _K * _y;
  _i = _i_prev + _K2 * (_e + _e_prev);
  _d = _K3 * _d_prev - _K4 * (_y - _y_prev);
  _ff = _Kff * _r;
  _aw = _p + _i + _d + _ff;

  //Serial.println("P: " + String(_p));
  //Serial.println("I: " + String(_i));
  //Serial.println("D: " + String(_d));
  Serial.println("FF: " + String(_ff));
  //Serial.println("AW: " + String(_aw));

  // Mudar valores para ter em conta a conversão lux2dc
  _saturate();
  Serial.println("U: " + String(_u));
  return _u;
}

void PID_Controller::flush(){
  _y_prev = _y;
  _e_prev = _e;
  _i_prev = _i;
  _d_prev = _d;
}
