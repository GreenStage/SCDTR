#ifndef I2C_H
#define I2C_H

#include "Arduino.h"

#define MAX_LEN 60

enum packet_id {
  PACKET_NONE = 0,
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

typedef struct send_single_float_ {
  uint8_t src;
  uint8_t dest;
  packet_id id;
  float val;
} __attribute__((__packed__)) single_float_packet;

//partilha o vector das iterações do consesus
typedef struct _send_multiple_float{
    uint8_t src;
    uint8_t dest;
    packet_id id;
    uint8_t n_data;
    float val[MAX_LEN];
} __attribute__((__packed__)) multiple_float_packet;

typedef struct send_single_byte_ {
  uint8_t src;
  uint8_t dest;
  packet_id id;
  uint8_t val;
}__attribute__((__packed__)) single_byte_packet;

//partilhar o vector da rede
typedef struct _network_resp_ {
    uint8_t src;
    uint8_t dest;
    packet_id id;
    uint8_t n_data;
    uint8_t val[MAX_LEN];
} __attribute__((__packed__)) network_packet;

typedef struct packet_t_ {
  uint8_t src;
  uint8_t dest;
  packet_id id;
} __attribute__((__packed__)) packet_t;

#define MAX_PACKET_SIZE sizeof(single_float_packet)

class i2c_controller {
  private:
    int _id;

  public:
    void init(void (*cb)(int));

    int getId();

    int sizeOfPacket(packet_id id);
    packet_id responseOf(packet_id id);

    void startSession(int to);
    void endSession();
    byte read();
    void write(packet_t *message, int size);
    void broadcast(packet_t *message);
    void send(int to, packet_t *message);
    void onReceive(void (*cb)(int));

    packet_t* createPacket(packet_id id, int src, int dest);
    single_byte_packet* createSingleBytePacket(packet_id id, int src, int dest, uint8_t val);
    single_float_packet* createSingleFloatPacket(packet_id id, int src, int dest, float val);
    multiple_float_packet* createMultiFloatPacket(packet_id id, int src, int dest, int n_data);
};

#endif
