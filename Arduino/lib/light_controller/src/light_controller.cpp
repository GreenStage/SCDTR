#include "light_controller.h"

const float V2L_A = pow(10, -LDR_OA/LDR_SL);
const float V2L_B = 1/LDR_SL;

// Default values
int T = 40;
int r = 40;
float Kp = 2.5*0.45;
float Ki = 1.2/T;
float Kaw = 1;

light_controller lc(T, r, Kp, Ki, Kaw);

volatile unsigned long currentTime;
volatile unsigned long lastTime = micros();
void light_interrupt(){
  currentTime = micros();
  if(currentTime-lastTime > 40000){
    lc.process();
    lastTime = currentTime;
  }
}

light_controller::light_controller(int T, int r, float Kp, float Ki, float Kaw) : _pid(T, r, Kp, Ki, Kaw) {
  _T = T;
  _ocupancy = 0;
}

float light_controller::_volt2ohm(float v_in){ return R0 * (VCC / v_in) - R0; }
float light_controller::_volt2lux(float v_in){ return V2L_A * pow(_volt2ohm(v_in), V2L_B); }
float light_controller::_getVolt(){
  float sum = 0;
  for(int i = 0; i < _n; i++)
    sum += analogRead(LDR_PIN);
  return sum / _n;
}

float light_controller::getErr(){ return _pid.getErr(); }
void light_controller::setLight(int dc){ analogWrite(LED_PIN, dc); }
float light_controller::getLight(){ return _volt2lux(_getVolt()); }
float light_controller::getIlluminance() { return _pid.getLight(); }
int light_controller::getDutyCycle(){ return _u; }

void light_controller::setMaxRef() { _ocupancy = 1; _r = 2*255/3; }
void light_controller::setMinRef() { _ocupancy = 0; _r = 255/3; }
int light_controller::getOcupancy() { return _ocupancy; }
float light_controller::getMaxRef() { return 2*255/3; }
float light_controller::getMinRef() { return 255/3; }
float light_controller::getRef() { return _r; }

void light_controller::calibrate(){
  setLight(0);
  delay(_T);
  _min_lux = getLight();
  setLight(255);
  delay(_T);
  _max_lux = getLight();
  _Kff = (_max_lux - _min_lux) / 255.0;
}

void light_controller::process(){
  _pid.setLight(getLight());
  _aw = _pid.process();
  if(_ff_mode){ _aw += _Kff*(_r-_min_lux); }
  _u = _pid.saturate(_aw);
  setLight(_u);
  if(_r != _pid.getRef()) _pid.setRef(_r);
  _pid.flush();
}

void light_controller::initInterrupt(){
  Timer1.initialize(_T);
  Timer1.attachInterrupt(light_interrupt);
}

void light_controller::startInterrupt(){ Timer1.start(); }

void light_controller::stopInterrupt(){ Timer1.stop(); }
