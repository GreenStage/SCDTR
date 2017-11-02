#include <Arduino.h>
#include <PID_Controller.h>

#define LDR_PIN A0
#define LED_PIN 9

#define R0 10000
#define VCC 1023
#define LDR_SL -0.691
#define LDR_OA 4.775

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
int r = 20;
int Kp = 17 * 0.45;
int Ki = 1.2/T;
int Kd = 0;
int a = 10;
int b = 1;

PID_Controller pid(T, r, Kp, Ki, Kd, a, b);

void setup() {
  Serial.begin(9600);  // start serial for output
}

// Loop variables
unsigned long startTime, endTime;
int in, out;
int incomingByte;

void loop() {
  startTime = millis();

  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    switch(incomingByte){
      case 49:
        pid.decRef(5);
        break;
      case 50:
        pid.incRef(5);
        break;
    }
  }

  in = getLight();
  pid.sample(volt2lux(in));

  out = pid.process();
  analogWrite(LED_PIN, out);

  pid.flush();

  endTime = millis();
  delay(T - (endTime - startTime));
}
