#include "r2a_controller.h"

r2a_controller::r2a_controller(state_controller &state, i2c_controller &i2c) : _state(state), _i2c(i2c) {}

message_id r2a_controller::_responseOf(message_id id){
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
    case RASP_M_DUTY_CICLE:      return ARD_M_DUTY_CICLE;
    case RASP_M_ILU:             return ARD_M_ILU;
    case RASP_TIME_RUNNING:      return ARD_TIME_RUNNING;
    default:                     return BASE;
  }
}

void r2a_controller::handleReceive(message_t *message){
  switch(message->id){
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
    case RASP_M_DUTY_CICLE:
    case RASP_M_ILU:
    case RASP_TIME_RUNNING:
      _state.addInMessage(message);
      break;
    case RASP_SET_NOT_OCCUP:
      _state.ocupancy = 0;
      break;
    case RASP_SET_OCCUP:
      _state.ocupancy = 1;
      break;
    case RASP_START_ILU:
      _state.L_stream_state = 1;
      break;
    case RASP_START_DUTY_CICLE:
      _state.D_stream_state = 1;
      break;
    case RASP_STOP_ILU:
      _state.L_stream_state = 0;
      break;
    case RASP_STOP_DUTY_CICLE:
      _state.D_stream_state = 0;
      break;
    case RASP_RESTART:
      _state.network_state = 0;
      _state.L_stream_state = 0;
      _state.D_stream_state = 0;
      break;
    default:
      break;
  }
}

void r2a_controller::handleResponse(message_t *message){
  int type = 0;
  int id = _i2c.getId();
  message_id m_id = _responseOf(message->id);

  byte_message_t *byte_res;
  float_message_t *float_res;
  multi_byte_message_t *multi_byte_res;
  multi_float_message_t *multi_float_res;

  switch(message->id){
    case RASP_ILU:
      type = 2;
      float_res = _i2c.singleFloatMessage(m_id, id, 0x48, _state.L);
      break;
    case RASP_DUTY_CICLE:
      type = 1;
      byte_res = _i2c.singleByteMessage(m_id, id, 0x48, _state.D);
      break;
    case RASP_LOWER_ILUMINANCE:
      type = 2;
      float_res = _i2c.singleFloatMessage(m_id, id, 0x48, _state.L);
      break;
    case RASP_ACC_ENERGY:
      type = 2;
      float_res = _i2c.singleFloatMessage(m_id, id, 0x48, _state.E);
      break;
    case RASP_ACC_CONFORT_ERR:
      type = 2;
      float_res = _i2c.singleFloatMessage(m_id, id, 0x48, _state.CE);
      break;
    case RASP_ACC_CONFORT_VAR:
      type = 2;
      float_res = _i2c.singleFloatMessage(m_id, id, 0x48, _state.CV);
      break;
    case RASP_POW_CONSUP:
      type = 2;
      float_res = _i2c.singleFloatMessage(m_id, id, 0x48, map(_state.D, 0, 255, 0, 100));
      break;
    case RASP_EXT_ILU:
      type = 2;
      float_res = _i2c.singleFloatMessage(m_id, id, 0x48, _state.O);
      break;
    case RASP_ILU_CTR:
      type = 2;
      float_res = _i2c.singleFloatMessage(m_id, id, 0x48, _state.R);
      break;
    case RASP_OCCUPANCY:
      type = 1;
      byte_res = _i2c.singleByteMessage(m_id, id, 0x48, _state.ocupancy);
      break;
    case RASP_M_ILU:
      type = 3;
      multi_float_res = _i2c.multiFloatMessage(m_id, id, 0x48, min(_state.ctr, 60));
      for(int i = 0; i < min(_state.ctr, 60); i++) multi_float_res->data[i] = _state.L_M[i];
      break;
    case RASP_M_DUTY_CICLE:
      type = 4;
      multi_byte_res = _i2c.multiByteMessage(m_id, id, 0x48, min(_state.ctr, 60));
      for(int i = 0; i < min(_state.ctr, 60); i++) multi_float_res->data[i] = _state.D_M[i];
      break;
    case RASP_TIME_RUNNING:
      type = 2;
      float_res = _i2c.singleFloatMessage(m_id, id, 0x48, millis() - _state.startTime);
      break;
    default:
      break;
  }

  switch (type) {
    case 1:
      _state.addOutMessage((message_t*) byte_res);
      break;
    case 2:
      _state.addOutMessage((message_t*) float_res);
      break;
    case 3:
      _state.addOutMessage((message_t*) multi_float_res);
      break;
    case 4:
      _state.addOutMessage((message_t*) multi_byte_res);
      break;
  }
}

void r2a_controller::process(){
  int id = _i2c.getId();
  // Luminosity Stream
  if(_state.L_stream_state) {
    _state.addOutMessage((message_t*)_i2c.singleFloatMessage(ARD_ILU, id, 0x48, _state.L));
  }
  // Dutycycle Stream
  if(_state.D_stream_state) {
    _state.addOutMessage((message_t*)_i2c.singleByteMessage(ARD_DUTY_CICLE, id, 0x48, _state.D));
  }
}
