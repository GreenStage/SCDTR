#include <Arduino.h>
#include <PID_Controller.h>

#define LDR_PIN A0
#define LED_PIN 9
#define BUTTON_PIN 8

#define R0 10000
#define VCC 1023
#define LDR_SL -0.691
#define LDR_OA 4.775
#define MAX_LUX 255

const int HIGH_REF = MAX_LUX * 2/3;
const int LOW_REF = MAX_LUX / 3;

#define N_STEPS 32

const float V2L_A = pow(10, -LDR_OA/LDR_SL);
const float V2L_B = 1/LDR_SL;

float volt2ohm(int v_in){ return R0 * (VCC / float(v_in)) - R0; }
float volt2lux(int v_in){ return V2L_A * pow(volt2ohm(v_in), V2L_B); }

int getLight(){
  int sum;
  for(int i = 0; i < 4; i++)
    sum += analogRead(LDR_PIN);
  return (sum / 4) + 0.5;
}

// Default values
int T = 50;
int r = 140;
int Kp = 10 * 0.6;
int Ki = 2/T;
int Kd = T/8;
int a = 10;
int b = 1;

PID_Controller pid(T, r, Kp, Ki, Kd, a, b);

void setup() {
  Serial.begin(9600);  // start serial for output
  pinMode(BUTTON_PIN, INPUT);
}

// Loop variables
unsigned long startTime, endTime;
int in, out;
int incoming;
int state = 0;
int newState, oldState;

void loop() {
  startTime = millis();

  in = getLight();
  pid.sample(volt2lux(in));

  out = pid.process();
  analogWrite(LED_PIN, out);

  newState = digitalRead(BUTTON_PIN);
  if(newState != oldState && oldState){
    state = state ? 0 : 1;
    pid.setRef(state ? HIGH_REF : LOW_REF);
  }
  oldState = newState;

  if (Serial.available() > 0) {
    // read the incoming byte:
    incoming = Serial.read();
    switch(incoming){
      case 49:
        pid.setRef(LOW_REF);
        break;
      case 50:
        pid.setRef(HIGH_REF);
        break;
      case 51:
        pid.decRef();
        break;
      case 52:
        pid.incRef();
        break;
    }
  }

  pid.flush();

  endTime = millis();
  delay(T - (endTime - startTime));
}
