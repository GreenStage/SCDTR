#include "Arduino.h"
#include "Light_Controller.h"
#include "PID_Controller.h"

const float V2L_A = pow(10, -LDR_OA/LDR_SL);
const float V2L_B = 1/LDR_SL;

// Default values
int T = 50;
int r = 20;
double Kp = 4.2;// 7*0.6;
double Ki = 0.05;//2.0/T;
double Kd = 6.125;//T/8.0;
int a = 10;
int b = 1;
int n = 5;

PID_Controller pid(T, r, Kp, Ki, Kd, a, b);

float Light_Controller::volt2ohm(int v_in){ return R0 * (VCC / float(v_in)) - R0; }
float Light_Controller::volt2lux(int v_in){ return V2L_A * pow(volt2ohm(v_in), V2L_B); }

int Light_Controller::getPeriod(){ return T; }

int Light_Controller::getLight(){
  int sum;
  for(int i = 0; i < n; i++)
    sum += analogRead(LDR_PIN);
  return round(sum / n);
}

void Light_Controller::calibrate(){
  analogWrite(LED_PIN, 0);
  delay(T);
  _min_lux = volt2lux(analogRead(LDR_PIN));
  analogWrite(LED_PIN, 255);
  delay(T);
  _max_lux = volt2lux(analogRead(LDR_PIN));
}

void Light_Controller::process(){
  pid.sample(volt2lux(getLight()));
  analogWrite(LED_PIN, pid.process());
  pid.flush();
}

void Light_Controller::setLowRef(){ pid.setRef(_max_lux/3); }
void Light_Controller::setHighRef(){ pid.setRef(2*_max_lux/3); }
