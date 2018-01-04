#include "i2c_controller.h"
#include "EEPROM.h"

int ID_ADDRESS = 0;
int i2c_controller::getId(){
  if(!_id){ _id = EEPROM.read(ID_ADDRESS); }
  return _id;
}

void i2c_controller::init(void (*cb)(int)){
  int id = getId();
  Wire.begin(id);
  TWAR = (id << 1) | 1;
  onReceive(cb);
}

void i2c_controller::onReceive(void (*cb)(int)){
  Wire.onReceive(cb);
}

int i2c_controller::messageSize(int type){
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

message_type i2c_controller::getMessageType(message_id id){
  switch(id){
    case BASE:
      return SIMPLE;
    default:
      return NULL;
  }
}

int size;
byte message_buffer[100];
message_t *message, *new_message;
message_t *i2c_controller::read(int numBytes){
  for(int i=0; i<numBytes; i++) message_buffer[i] = Wire.read();
  message = (message_t*) message_buffer;

  size = messageSize(getMessageType(message->id));

  new_message = (message_t*) malloc(size);
  for(int i=0; i<size; i++) new_message[i] = message[i];
  return new_message;
}


void i2c_controller::broadcast(message_t *message){
  send(0, message);
}

void i2c_controller::send(int to, message_t *message){
  startSession(to);
  write(message, messageSize(message->id));
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
