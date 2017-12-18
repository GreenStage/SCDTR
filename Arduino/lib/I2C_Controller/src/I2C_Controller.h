#ifndef I2C_H
#define I2C_H

#include "Arduino.h"

#define MAX_LEN 30

enum packet_ids {
  PACKET_NONE = 0,
  RASP_REQ_MIN = 0x40,
  RASP_REQ_ILU,
  RASP_REQ_DUTY_CICLE,
  RASP_REQ_LOWER_ILUMINANCE,
  RASP_REQ_ACC_ENERGY,
  RASP_REQ_ACC_CONFORT_ERR,
  RASP_REQ_ACC_CONFORT_VAR,
  RASP_REQ_POW_CONSUP,
  RASP_REQ_EXT_ILU,
  RASP_REQ_ILU_CTR,
  RASP_REQ_OCCUPANCY,
  RASP_REQ_SET_NOT_OCCUP,
  RASP_REQ_SET_OCCUP,
  //NEW PACKETS -- Last Minute Buffers
  RASP_REQ_M_ILU,
  RASP_REQ_M_DUTY_CICLE,
  //NEW PACKETS -- Stream requests
  RASP_REQ_START_ILU,
  RASP_REQ_START_DUTY_CICLE,
  RASP_REQ_STOP_ILU,
  RASP_REQ_STOP_DUTY_CICLE,
  //NEW PACKETS -- Restart requests
  RASP_REQ_RESTART,
  RASP_REQ_TIME_RUNNING,
  RASP_REQ_MAX,

  ARD_REQ_MIN = 0x80,
  //ARDUINO_TO_ARDUINO REQUEST PACKETS
  ARD_REQ_MAX,

  //ARDUINO RESONSE PACKETS
  ARD_RES_MIN = 0xC0,
  ARD_RES_NETWORK,
  ARD_RES_ILU,
  ARD_RES_DUTY_CICLE,
  ARD_RES_LOWER_ILUMINANCE,
  ARD_RES_ACC_ENERGY,
  ARD_RES_ACC_CONFORT_ERR,
  ARD_RES_ACC_CONFORT_VAR,
  ARD_RES_POW_CONSUP,
  ARD_RES_EXT_ILU,
  ARD_RES_ILU_CTR,
  ARD_RES_OCCUPANCY,
  //NEW PACKETS -- Last Minute Buffers
  ARD_RES_M_ILU,
  ARD_RES_M_DUTY_CICLE,
  //NEW PACKETS -- Restart requests
  ARD_RES_TIME_RUNNING,
  //Afonso
  ARD_RES_BC_ADDR,
  ARD_RES_DC,
  ARD_RES_INIT_CAL,

  ARD_RES_MAX
};

typedef struct send_single_float_ {
  uint8_t src;
  uint8_t dest;
  uint8_t id;
  float val;
} __attribute__((__packed__)) single_float_package;

typedef struct send_single_byte_ {
  uint8_t src;
  uint8_t dest;
  uint8_t id;
  uint8_t val;
}__attribute__((__packed__)) single_byte_package;

//partilhar o vector da rede
typedef struct _network_resp_{
    uint8_t src;
    uint8_t dest;
    uint8_t type;
    uint8_t n_data;
    uint8_t data[MAX_LEN];
} __attribute__((__packed__)) network_packet;

//partilha o vector das iterações do consesus
typedef struct _send_multiple_float{
    uint8_t src;
    uint8_t dest;
    uint8_t type;
    uint8_t n_data;
    float data[MAX_LEN];
} __attribute__((__packed__)) multiple_float_packet;

typedef struct packet_t_{
  uint8_t src;
  uint8_t dest;
  uint8_t id;
} __attribute__((__packed__)) packet_t;

template<class T, size_t N>
constexpr size_t size(T (&)[N]) { return N; }
#define MAX_PACKET_SIZE sizeof(single_float_package)

class i2c_controller {
  private:
    int _id;
    int _net[];
    int _sync;

  public:
    void init(void (*cb)(int));

    int getId();
    int *getNet();

    int sizeOfPacket(packet_t * p);
    int responseOf(int id);

    byte read();
    void sync();
    void broadcast(packet_t *message);
    void send(int id, packet_t *message);
    void onReceive(void (*cb)(int));
};

#endif
