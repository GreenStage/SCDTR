#ifndef LIGHT_H
#define LIGHT_H

#define LDR_PIN A0
#define LED_PIN 9

#define R0 10000
#define VCC 1023

#define LDR_SL -0.691
#define LDR_OA 4.775

#define N_STEPS 128

#define CALIBRATING 2
#define READY 1

class LightController {
  private:
    void clockCallback();
    float volt2ohm(int v_in);
    float volt2lux(int v_in);
    float lux2dc(float lux);

  public:
    void calibrate();
    void setLightRef(int r);
};

#endif
