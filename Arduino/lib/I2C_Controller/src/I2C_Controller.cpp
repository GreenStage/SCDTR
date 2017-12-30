#include "i2c_controller.h"
#include "EEPROM.h"

int ID_ADDRESS = 0;
int i2c_controller::getId(){
  // Get id from eeprom if it doesn't already exist
  if(!_id){ _id = EEPROM.read(ID_ADDRESS); }
  return _id;
}

void i2c_controller::init(void (*cb)(int)){
  int id = getId();
  // Start wire communications
  Wire.begin(id);
  // Listen to broadcast
  TWAR = (id << 1) | 1;
  // Register received message callback
  onReceive(cb);
}

void i2c_controller::onReceive(void (*cb)(int)){
  Wire.onReceive(cb);
}

// Get message size from type
int i2c_controller::messageSize(message_type type){
  switch(type){
    case MULTI_FLOAT:
      return sizeof(multi_float_message_t);
    case MULTI_BYTE:
      return sizeof(multi_byte_message_t);
    case SIMPLE:
      return sizeof(message_t);
    case FLOAT:
      return sizeof(float_message_t);
    case BYTE:
      return sizeof(byte_message_t);
    default:
      return 0;
  }
}

// Get message type from id
message_type i2c_controller::messageType(message_id id){
  switch(id){
    case BASE:
    case RASP_ILU:
    case RASP_DUTY_CICLE:
    case RASP_LOWER_ILUMINANCE:
    case RASP_ACC_ENERGY:
    case RASP_ACC_CONFORT_ERR:
    case RASP_ACC_CONFORT_VAR:
    case RASP_POW_CONSUP:
    case RASP_EXT_ILU:
    case RASP_ILU_CTR:
    case RASP_OCCUPANCY:
    case RASP_SET_NOT_OCCUP:
    case RASP_SET_OCCUP:
    case RASP_M_ILU:
    case RASP_M_DUTY_CICLE:
    case RASP_START_ILU:
    case RASP_START_DUTY_CICLE:
    case RASP_STOP_ILU:
    case RASP_STOP_DUTY_CICLE:
    case RASP_RESTART:
    case RASP_TIME_RUNNING:
    case RASP_MAX:
    case ARD_ADDR:
    case ARD_SYNC:
    case ARD_INIT_CAL:
    case ARD_HELLO:
      return SIMPLE;
    case ARD_ILU:
    case ARD_EXT_ILU:
    case ARD_ILU_CTR:
    case ARD_POW_CONSUP:
    case ARD_ACC_ENERGY:
    case ARD_TIME_RUNNING:
    case ARD_ACC_CONFORT_VAR:
    case ARD_ACC_CONFORT_ERR:
    case ARD_LOWER_ILUMINANCE:
      return FLOAT;
    case ARD_OCCUPANCY:
    case ARD_DUTY_CICLE:
    case ARD_DC:
      return BYTE;
    case ARD_M_ILU:
      return MULTI_FLOAT;
    case ARD_NETWORK:
    case ARD_CONSENSUS:
    case ARD_M_DUTY_CICLE:
      return MULTI_BYTE;
    default:
      return NULL;
  }
}

int size;
byte message_buffer[100];
message_t *message, *new_message;
message_t *i2c_controller::read(int numBytes){
  // Read message from bus to buffer
  for(int i=0; i<numBytes; i++) message_buffer[i] = Wire.read();
  message = (message_t*) message_buffer;
  
  // Get message size
  size = messageSize(messageType(message->id));

  // Allocate space for message
  new_message = (message_t*) malloc(size);

  // Copy message
  for(int i=0; i<size; i++) new_message[i] = message[i];
  return new_message;
}


void i2c_controller::broadcast(message_t *message){
  send(0, message);
}

void i2c_controller::send(int to, message_t *message){
  startSession(to);
  write(message, messageSize(messageType(message->id)));
  endSession();
}

void i2c_controller::write(message_t *message, int size){
  Wire.write((byte*) message, size);
}

void i2c_controller::startSession(int to){
  Wire.beginTransmission(to);
}

void i2c_controller::endSession(){
  Wire.endTransmission();
}

message_t *i2c_controller::simpleMessage(message_id id, int src, int dest){
  message_t *message = (message_t*) malloc(sizeof(message_t));
  message->id = id;
  message->src = src;
  message->dest = dest;
  return message;
}

byte_message_t *i2c_controller::singleByteMessage(message_id id, int src, int dest, uint8_t data){
  byte_message_t *message = (byte_message_t*) malloc(sizeof(byte_message_t));
  message->id = id;
  message->src = src;
  message->dest = dest;
  message->data = data;
  return message;
}

float_message_t *i2c_controller::singleFloatMessage(message_id id, int src, int dest, float data){
  float_message_t *message = (float_message_t*) malloc(sizeof(float_message_t));
  message->id = id;
  message->src = src;
  message->dest = dest;
  message->data = data;
  return message;
}

multi_byte_message_t *i2c_controller::multiByteMessage(message_id id, int src, int dest, int size){
  multi_byte_message_t *message = (multi_byte_message_t*) malloc(sizeof(multi_byte_message_t)-(MAX_LEN-size)*sizeof(uint8_t));
  message->id = id;
  message->src = src;
  message->dest = dest;
  message->len = size;
  return message;
}

multi_float_message_t *i2c_controller::multiFloatMessage(message_id id, int src, int dest, int size){
  multi_float_message_t *message = (multi_float_message_t*) malloc(sizeof(multi_float_message_t)-(MAX_LEN-size)*sizeof(float));
  message->id = id;
  message->src = src;
  message->dest = dest;
  message->len = size;
  return message;
}
