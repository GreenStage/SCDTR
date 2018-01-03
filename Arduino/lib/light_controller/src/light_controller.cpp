#include "light_controller.h"

const float V2L_A = pow(10, -LDR_OA/LDR_SL);
const float V2L_B = 1/LDR_SL;

// Default values
int T = 40;
int r = 70;
float Kp = 2.5*0.45;
float Ki = 1.2/T;
float Kaw = 1;

light_controller::light_controller(state_controller &state) : _pid(T, r, Kp, Ki, Kaw), _state(state) {}

float light_controller::_volt2ohm(float v){ return R0 * (VCC / v) - R0; }
float light_controller::_volt2lux(float v){ return V2L_A * pow(_volt2ohm(v), V2L_B); }
float light_controller::_getVolt(){
  float sum = 0;
  for(int i = 0; i < _state.N; i++)
    sum += analogRead(LDR_PIN);
  return sum / _state.N;
}

void light_controller::setLight(int u){ analogWrite(LED_PIN, u); }
float light_controller::getLight(){ return _volt2lux(_getVolt()); }

void light_controller::calibrate(){
  setLight(0);
  delay(_state.T);
  _state.O = getLight();
  setLight(255);
  delay(_state.T);
  _state.M = getLight();
  _state.Kff = (_state.M - _state.O) / 255.0;
}

void light_controller::process(){
  _state.L = getLight();
  _pid.setLight(_state.L);
  _state.AW = _pid.process();
  if(_state.ff_mode){ _state.AW += _state.Kff*(_state.R-_state.O); }
  _state.D = _pid.saturate(_state.AW);
  setLight(_state.D);
  if(_state.R != _pid.getRef()) _pid.setRef(_state.R);
  _state.E = _pid.getErr();
  _pid.flush();
}

void light_controller::init(void (*cb)()){
  Timer1.initialize(_state.T);
  Timer1.attachInterrupt(cb);
}
