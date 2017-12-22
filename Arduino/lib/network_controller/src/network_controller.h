#ifndef SERIAL_H
#define SERIAL_H

#include "Arduino.h"
#include "QueueArray.h"
#include "CircularBuffer.h"
#include "stdint.h"
#include "light_controller.h"
#include "i2c_controller.h"

class network_controller {
  private:
    light_controller& _lc;
    i2c_controller& _i2c;
    int *_net;
    int _net_size = 0;
    int _net_index;
    int _ctr = 0;
    int _sync_ctr = 0;
    int _network_state = 0; // 0 - uninitialized; 1 - initializing; 2 - initilized
    int _consensus_state = 0;
    int _calibration_state = 0; // 0 - Uncalibrated; 1 - calibrating; 2 - calibrated
    int _L_stream_state = 0; // 0 - off; 1 - on
    int _D_stream_state = 0; // 0 - off; 1 - on
    int _U;

    double _R, _O;
    double *_K;
    double _L, _D;
    CircularBuffer<double, 60> _L_M, _D_M;

    double _E = 0;
    double _CE = 0;
    double _CV = 0;
    unsigned long _startTime = millis();
    unsigned long _currentTime;
    unsigned long _lastTime;
    unsigned long _timeout;

    float *_dc_copy;
    bool _dc_flag;

    void _sync();
    void _insert(int id);

  public:
    network_controller(light_controller &lc, i2c_controller &i2c);

    void calibrate();
    void consensus();
    void process();

    int raspberry_request(packet_t* req, packet_t** res);
    int arduino_request(packet_t* req);
    void updateDcCopy(float data[MAX_LEN], int len);
    void init();
};

#endif
