#include "Arduino.h"
#include "LightController.h"
#include "PID_Controller.h"

const float V2L_A = pow(10, -LDR_OA/LDR_SL);
const float V2L_B = 1/LDR_SL;

// Default values
int T = 50;
int r = 20;
int Kp = 10;
int Ki = 10;
int Kd = 1;
int a = 10;
int b = 1;

PID_Controller pid(T, r, Kp, Ki, Kd, a, b);

Scheduler runner;
int ready = false;
unsigned long startTime, endTime;
void clockCallback(){
  if(ready){
    startTime = millis();
    in = analogRead(LDR_PIN);
    pid.sample(volt2lux(in));

    out = lux2dc(pid.process());
    analogWrite(LED_PIN, out);

    pid.flush();
    endTime = millis();
    delay(T - (endTime - startTime));
  }
  delay(T);
}


void addTask(parent){
  Task lightTask(T, -1, &clockCallback, parent);
}

float LightController::volt2ohm(int v_in){ return R0 * (VCC / float(v_in)) - R0; }

float LightController::volt2lux(int v_in){ return V2L_A * pow(volt2ohm(v_in), V2L_B); }

float L2DC_OA, L2DC_SL;

void LightController::calibrate(){
  int step = 256 / ( N_STEPS );
  float n, x, y;
  float Sx, Sy, Sxy, Sxx;

  Sx = Sxy = Sxx = 0;

  for(x = 0, n = 0; x < 256; x += step, n++) {
    analogWrite(LED_PIN, x);
    Sx += x;
    Sxx += x*x;
    delay(50);
    y = volt2lux(analogRead(LDR_PIN));

    Sy += y;
    Sxy += x*y;
  }

  L2DC_SL = (n*Sxy - Sx*Sy) / (n*Sxx - Sx*Sx);
  L2DC_OA = (Sy/n) - L2DC_SL * (Sx/n);

  Serial.println("Lin Reg: y =" + String(L2DC_SL) + " * x + " + String(L2DC_OA));
}

float LightController::lux2dc(float lux){ return (lux - L2DC_OA) / L2DC_SL; }
