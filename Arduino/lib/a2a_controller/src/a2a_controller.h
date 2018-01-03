#ifndef A2A_H
#define A2A_H

#include "Arduino.h"
#include "i2c_controller.h"
#include "state_controller.h"
#include "light_controller.h"

class a2a_controller {
  private:
    i2c_controller& _i2c;
    light_controller& _lc;
    state_controller& _state;

    int _sync_ctr = 0;
    void _sync();

  public:
    a2a_controller(state_controller& state, i2c_controller& i2c, light_controller& lc);

    void handleReceive(message_t *message);
    void handleResponse(message_t *message);

    void handleNetAddr(message_t *message);
    void handleNetHello(message_t *message);
    void handleConsensus(multi_float_message_t *message);

    void initNetwork();
    void calibrate();
    void consensus();

    void printNet();
    void printCalib();
    void printConse(float * d);

    void process();
};

#endif
