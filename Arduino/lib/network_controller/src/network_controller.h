#ifndef SERIAL_H
#define SERIAL_H

#include "Arduino.h"
#include "QueueArray.h"
#include "stdint.h"
#include "light_controller.h"
#include "i2c_controller.h"

class network_controller {
  private:
    light_controller& _lc;
    i2c_controller& _i2c;
    float *_dc_copy;
    bool _dc_flag, _cal_flag;
    int *_net;
    double *_K;
    double _O;

  public:
    network_controller(light_controller &lc, i2c_controller &i2c);

    void calibrate();
    void consensus();
    void process();

    int raspberry_request(packet_t* req, packet_t* res);
    int arduino_request(packet_t* req, packet_t* res);
    int arduino_response(packet_t* req, packet_t* res);

    void init();
};

#endif
