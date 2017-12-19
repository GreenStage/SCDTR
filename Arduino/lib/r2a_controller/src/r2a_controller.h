#ifndef R2A_H
#define R2A_H

#include "Arduino.h"
#include "i2c_controller.h"

class r2a_controller {
  private:
    i2c_controlller &_i2c;
  public:
    r2a_controller(i2c_controller i2c);
    int process(packet_t* req, packet_t** res);
};

#endif
