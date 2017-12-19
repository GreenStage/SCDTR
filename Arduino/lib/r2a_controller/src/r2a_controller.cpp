#include "r2a_controller.h"

r2a_controller::r2a_controller(i2c_controller i2c) : _i2c(i2c) {}

int r2a_controller::process(packet_t* req, packet_t** res){
  int type = 0;
  int id = _i2c.getId();
  single_byte_packet *byte_res;
  single_float_packet *float_res;
  multiple_float_packet *multi_float_res;
  packet_id p_id = _i2c.responseOf(req->id);
  switch(req->id){
    case RASP_ILU:
      type = 2;
      float_res = _i2c.createSingleFloatPacket(p_id, id, 0x69, _L);
      break;
    case RASP_DUTY_CICLE:
      type = 1;
      byte_res = _i2c.createSingleBytePacket(p_id, id, 0x69, _D);
      break;
    case RASP_LOWER_ILUMINANCE:
      type = 2;
      float_res = _i2c.createSingleFloatPacket(p_id, id, 0x69, _L);
      break;
    case RASP_ACC_ENERGY:
      type = 2;
      float_res = _i2c.createSingleFloatPacket(p_id, id, 0x69, _E);
      break;
    case RASP_ACC_CONFORT_ERR:
      type = 2;
      float_res = _i2c.createSingleFloatPacket(p_id, id, 0x69, _CE);
      break;
    case RASP_ACC_CONFORT_VAR:
      type = 2;
      float_res = _i2c.createSingleFloatPacket(p_id, id, 0x69, _CV);
      break;
    case RASP_POW_CONSUP:
      type = 2;
      float_res = _i2c.createSingleFloatPacket(p_id, id, 0x69, map(_D, 0, 255, 0, 100));
      break;
    case RASP_EXT_ILU:
      type = 2;
      float_res = _i2c.createSingleFloatPacket(p_id, id, 0x69, _O);
      break;
    case RASP_ILU_CTR:
      type = 2;
      float_res = _i2c.createSingleFloatPacket(p_id, id, 0x69, _lc.getRef());
      break;
    case RASP_OCCUPANCY:
      type = 1;
      byte_res = _i2c.createSingleBytePacket(p_id, id, 0x69, _lc.getOcupancy());
      break;
    case RASP_SET_NOT_OCCUP:
      type = 0;
      _lc.setMinRef();
      break;
    case RASP_SET_OCCUP:
      type = 0;
      _lc.setMaxRef();
      break;
    case RASP_M_ILU:
      type = 3;
      multi_float_res = _i2c.createMultiFloatPacket(p_id, id, 0x69, min(_ctr, 60));
      for(int i = 0; i < min(_ctr, 60); i++) multi_float_res->val[i] = _L_M[i];
      break;
    case RASP_M_DUTY_CICLE:
      type = 3;
      multi_float_res = _i2c.createMultiFloatPacket(p_id, id, 0x69, min(_ctr, 60));
      for(int i = 0; i < min(_ctr, 60); i++) multi_float_res->val[i] = _D_M[i];
      break;
    case RASP_START_ILU:
      _L_stream_state = 1;
      break;
    case RASP_START_DUTY_CICLE:
      _D_stream_state = 1;
      break;
    case RASP_STOP_ILU:
      _L_stream_state = 0;
      break;
    case RASP_STOP_DUTY_CICLE:
      _D_stream_state = 0;
      break;
    case RASP_RESTART:
      _network_state = 0;
      break;
    case RASP_TIME_RUNNING:
      type = 2;
      float_res = _i2c.createSingleFloatPacket(p_id, id, 0x69, millis() - _startTime);
      break;
    default:
      break;
  }

  if(send_bytes){
    switch(type){
      case 1:
        (*res) = (packet_t*) byte_res;
        break;
      case 2:
        (*res) = (packet_t*) float_res;
        break;
      case 3:
        (*res) = (packet_t*) multi_float_res;
        break;
    }
  }
  return send_bytes;
}
