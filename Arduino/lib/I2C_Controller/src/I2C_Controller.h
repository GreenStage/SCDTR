
#ifndef I2C_H
#define I2C_H

#include "Arduino.h"
#include "Wire.h"

#define MAX_LEN 100

enum message_id {
  BASE = 0,

  RASP_MIN = 0x40,
  RASP_ILU,
  RASP_DUTY_CICLE,
  RASP_LOWER_ILUMINANCE,
  RASP_ACC_ENERGY,
  RASP_ACC_CONFORT_ERR,
  RASP_ACC_CONFORT_VAR,
  RASP_POW_CONSUP,
  RASP_EXT_ILU,
  RASP_ILU_CTR,
  RASP_OCCUPANCY,
  RASP_SET_NOT_OCCUP,
  RASP_SET_OCCUP,
  RASP_M_ILU,
  RASP_M_DUTY_CICLE,
  RASP_START_ILU,
  RASP_START_DUTY_CICLE,
  RASP_STOP_ILU,
  RASP_STOP_DUTY_CICLE,
  RASP_RESTART,
  RASP_TIME_RUNNING,
  RASP_MAX,

  ARD_MIN = 0x80,
  ARD_NETWORK,
  ARD_ILU,
  ARD_DUTY_CICLE,
  ARD_LOWER_ILUMINANCE,
  ARD_ACC_ENERGY,
  ARD_ACC_CONFORT_ERR,
  ARD_ACC_CONFORT_VAR,
  ARD_POW_CONSUP,
  ARD_EXT_ILU,
  ARD_ILU_CTR,
  ARD_OCCUPANCY,
  ARD_M_ILU,
  ARD_M_DUTY_CICLE,
  ARD_TIME_RUNNING,
  ARD_ADDR,
  ARD_DC,
  ARD_INIT_CAL,
  ARD_SYNC,
  ARD_CONSENSUS,
  ARD_MAX
};

enum message_type {
  MULTI_FLOAT,
  MULTI_BYTE,
  SIMPLE,
  FLOAT,
  BYTE
};

typedef struct multi_float_message_t_ {
  uint8_t src;
  uint8_t dest;
  message_id id;
  uint8_t len;
  float data[MAX_LEN];
} __attribute__((__packed__)) multi_float_message_t;

typedef struct multi_byte_message_t_ {
  uint8_t src;
  uint8_t dest;
  message_id id;
  uint8_t len;
  uint8_t data[MAX_LEN];
} __attribute__((__packed__)) multi_byte_message_t;

typedef struct float_message_t_ {
  uint8_t src;
  uint8_t dest;
  message_id id;
  float data;
} __attribute__((__packed__)) float_message_t;

typedef struct byte_message_t_ {
  uint8_t src;
  uint8_t dest;
  message_id id;
  uint8_t data;
} __attribute__((__packed__)) byte_message_t;

typedef struct message_t_ {
  uint8_t src;
  uint8_t dest;
  message_id id;
} __attribute__((__packed__)) message_t;

class i2c_controller {
  private:
    int _id;

  public:
    int getId();

    int messageSize(int type);

    void init(void (*cb)(int));
    void onReceive(void (*cb)(int));
    message_type getMessageType(message_id id);

    void startSession(int to);
    void endSession();

    message_t* read(int numBytes);
    void write(message_t *message, int size);

    void broadcast(message_t *message);
    void send(int to, message_t *message);

    message_t* simpleMessage(message_id id, int src, int dest);
    byte_message_t* singleByteMessage(message_id id, int src, int dest, uint8_t data);
    float_message_t* singleFloatMessage(message_id id, int src, int dest, float data);
    multi_byte_message_t* multiByteMessage(message_id id, int src, int dest, int size);
    multi_float_message_t* multiFloatMessage(message_id id, int src, int dest, int size);
};

#endif
