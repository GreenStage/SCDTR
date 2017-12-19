#include "Arduino.h"
#include "EEPROM.h"
#include "Wire.h"
#include "i2c_controller.h"

i2c_controller i2c;

void i2c_controller::init(void (*cb)(int)){
  TWAR = (getId() << 1) | 1;
  onReceive(cb);
}

int ID_ADDRESS = 0;
int i2c_controller::getId(){
  if(!_id){ _id = EEPROM.read(ID_ADDRESS); }
  return _id;
}

byte i2c_controller::read(){ return Wire.read(); }

void i2c_controller::broadcast(packet_t *message){
  send(0, message);
}

void i2c_controller::send(int to, packet_t *message){
  startSession(to);
  write(message);
  endSession();
}

void i2c_controller::write(packet_t *message){
  Wire.write((byte*) message, sizeOfPacket(message->id));
}

void i2c_controller::startSession(int to){
  Wire.beginTransmission(to);
}

void i2c_controller::endSession(){
  Wire.endTransmission();
}

void i2c_controller::onReceive(void (*cb)(int)){
  Wire.onReceive(cb);
}

packet_id i2c_controller::responseOf(packet_id id){
  switch(id){
    case RASP_ILU:               return ARD_ILU;
    case RASP_DUTY_CICLE:        return ARD_DUTY_CICLE;
    case RASP_LOWER_ILUMINANCE:  return ARD_LOWER_ILUMINANCE;
    case RASP_ACC_ENERGY:        return ARD_ACC_ENERGY;
    case RASP_ACC_CONFORT_ERR:   return ARD_ACC_CONFORT_ERR;
    case RASP_ACC_CONFORT_VAR:   return ARD_ACC_CONFORT_VAR;
    case RASP_POW_CONSUP:        return ARD_POW_CONSUP;
    case RASP_EXT_ILU:           return ARD_EXT_ILU;
    case RASP_ILU_CTR:           return ARD_ILU_CTR;
    case RASP_OCCUPANCY:         return ARD_OCCUPANCY;
    default:                     return PACKET_NONE;
  }
}

int i2c_controller::sizeOfPacket(packet_id id){
  int packet_size = sizeof(packet_t);
  int single_byte = sizeof(single_byte_packet);
  int single_float = sizeof(single_float_packet);
  int multi_float = sizeof(multiple_float_packet);
  switch(id){
    case ARD_M_ILU:
    case ARD_M_DUTY_CICLE:
      return multi_float;
    case ARD_ILU:
    case ARD_LOWER_ILUMINANCE:
    case ARD_ACC_ENERGY:
    case ARD_ILU_CTR:
    case ARD_ACC_CONFORT_ERR:
    case ARD_ACC_CONFORT_VAR:
    case ARD_POW_CONSUP:
    case ARD_EXT_ILU:
    case ARD_TIME_RUNNING:
      return single_float;
    case ARD_DUTY_CICLE:
    case ARD_OCCUPANCY:
      return single_byte;
    case RASP_ILU:
    case RASP_DUTY_CICLE:
    case RASP_ACC_ENERGY:
    case RASP_LOWER_ILUMINANCE:
    case RASP_ACC_CONFORT_ERR:
    case RASP_ACC_CONFORT_VAR:
    case RASP_POW_CONSUP:
    case RASP_EXT_ILU:
    case RASP_ILU_CTR:
    case RASP_OCCUPANCY:
    case RASP_RESTART:
    case RASP_SET_NOT_OCCUP:
    case RASP_SET_OCCUP:
    case RASP_M_ILU:
    case RASP_M_DUTY_CICLE:
    case RASP_START_ILU:
    case RASP_START_DUTY_CICLE:
    case RASP_STOP_ILU:
    case RASP_STOP_DUTY_CICLE:
    case RASP_TIME_RUNNING:
    case ARD_INIT_CAL:
    case ARD_SYNC:
    case ARD_ADDR:
    case PACKET_NONE:
    default:
      return packet_size;
  }
}

packet_t* i2c_controller::createPacket(packet_id id, int src, int dest){
  packet_t *p = (packet_t*) malloc(sizeof(packet_t));
  p->id = id;
  p->src = src;
  p->dest = dest;
  return p;
}

single_byte_packet* i2c_controller::createSingleBytePacket(packet_id id, int src, int dest, uint8_t val){
  single_byte_packet *p = (single_byte_packet*) malloc(sizeof(single_byte_packet));
  p->id = id;
  p->src = src;
  p->dest = dest;
  p->val = val;
  return p;
}

single_float_packet* i2c_controller::createSingleFloatPacket(packet_id id, int src, int dest, float val){
  single_float_packet *p = (single_float_packet*) malloc(sizeof(single_float_packet));
  p->id = id;
  p->src = src;
  p->dest = dest;
  p->val = val;
  return p;
}

multiple_float_packet* i2c_controller::createMultiFloatPacket(packet_id id, int src, int dest, int n_data){
  multiple_float_packet *p = (multiple_float_packet*) malloc(sizeof(multiple_float_packet));
  p->id = id;
  p->src = src;
  p->dest = dest;
  return p;
}
