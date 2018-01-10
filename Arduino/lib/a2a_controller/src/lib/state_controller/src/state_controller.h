#ifndef STATE_H
#define STATE_H

#include "Arduino.h"
#include "QueueArray.h"
#include "CircularBuffer.h"
#include "i2c_controller.h"

class state_controller {
  private:
    QueueArray<message_t*> _in_messages, _out_messages;

    void _processMetrics();

  public:
    int *net;
    int net_index;
    int net_size = 0;

    int L_stream_state = 0; // 0 - off; 1 - on
    int D_stream_state = 0; // 0 - off; 1 - on

    int network_state = 0; // 0 - uninitialized; 1 - initializing; 2 - initilized
    int consensus_state = 0; // 0 - unexchanged; 1 - exchanging; 2 - exchanged
    int calibration_state = 0; // 0 - Uncalibrated; 1 - calibrating; 2 - calibrated

    int D, N, T;
    float *K, Kff, AW;
    float R, M, O, err;
    float L, E, CE, CV;

    CircularBuffer<int, 60> D_M;
    CircularBuffer<float, 60> L_M;

    int ctr = 0;

    unsigned long timeout;
    unsigned long startTime = millis();
    unsigned long currentTime, lastTime;

    bool ocupancy;
    bool ff_mode;

    int *dc_copy;
    bool dc_flag;

    bool hasInMessages();
    bool hasOutMessages();

    message_t *getInMessage();
    message_t *getOutMessage();

    void addInMessage(message_t *message);
    void addOutMessage(message_t *message);

    void addNodeToNetwork(int id);

    void init();
    void process();
};

#endif
