#ifndef R2A_H
#define R2A_H

#include "Arduino.h"
#include "i2c_controller.h"
#include "state_controller.h"

class r2a_controller {
  private:
    i2c_controller &_i2c;
    state_controller &_state;

    message_id _responseOf(message_id id);

  public:
    r2a_controller(state_controller &state, i2c_controller &i2c);

    void handleReceive(message_t *message);
    void handleResponse(message_t *message);

    void process();
};

#endif
