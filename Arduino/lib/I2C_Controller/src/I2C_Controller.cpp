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

int *i2c_controller::getNet(){ return _net; }

byte i2c_controller::read(){ return Wire.read(); }

void i2c_controller::sync(){
}

void i2c_controller::broadcast(packet_t *message){
  send(0, message);
}

void i2c_controller::send(int to, packet_t* message){
  Wire.beginTransmission(to);
  Wire.write((byte*) message, sizeOfPacket(message));
  Wire.endTransmission();
}

void i2c_controller::onReceive(void (*cb)(int)){
  Wire.onReceive(cb);
}

int i2c_controller::sizeOfPacket(packet_t *p){
  switch(p->id){
    case ARDUINO_RES_ILU:
    case ARDUINO_RES_DUTY_CICLE:
    case ARDUINO_RES_LOWER_ILUMINANCE:
    case ARDUINO_RES_ACC_ENERGY:
    case ARDUINO_RES_ACC_CONFORT_ERR:
    case ARDUINO_RES_ACC_CONFORT_VAR:
    case ARDUINO_RES_POW_CONSUP:
    case ARDUINO_RES_EXT_ILU:
    case ARDUINO_RES_ILU_CTR:
      return sizeof(single_float_package);
    case ARDUINO_RES_OCCUPANCY:
      return sizeof(single_byte_package);
    default:
      return 0;
  }
}

int i2c_controller::responseOf(int id){
  switch(id){
    case RASP_REQ_ILU:               return ARDUINO_RES_ILU;
    case RASP_REQ_DUTY_CICLE:        return ARDUINO_RES_DUTY_CICLE;
    case RASP_REQ_LOWER_ILUMINANCE:  return ARDUINO_RES_LOWER_ILUMINANCE;
    case RASP_REQ_ACC_ENERGY:        return ARDUINO_RES_ACC_ENERGY;
    case RASP_REQ_ACC_CONFORT_ERR:   return ARDUINO_RES_ACC_CONFORT_ERR;
    case RASP_REQ_ACC_CONFORT_VAR:   return ARDUINO_RES_ACC_CONFORT_VAR;
    case RASP_REQ_POW_CONSUP:        return ARDUINO_RES_POW_CONSUP;
    case RASP_REQ_EXT_ILU:           return ARDUINO_RES_EXT_ILU;
    case RASP_REQ_ILU_CTR:           return ARDUINO_RES_ILU_CTR;
    case RASP_REQ_OCCUPANCY:         return ARDUINO_RES_OCCUPANCY;
    default:                         return PACKET_NONE;
  }
}
