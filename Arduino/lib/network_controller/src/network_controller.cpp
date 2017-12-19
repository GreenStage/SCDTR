#include "network_controller.h"

extern light_controller lc;
extern i2c_controller i2c;
network_controller nc(lc, i2c);

network_controller::network_controller(light_controller& lc, i2c_controller& i2c) : _lc(lc), _i2c(i2c) {
  _dc_flag = false;
}

QueueArray<packet_t*> in_messages, out_messages;
packet_t *in_message, *out_message;

int read_bytes, send_bytes;
packet_t *in_buffer, *out_buffer;
void onReceive(int numBytes){
  for(read_bytes = 0; read_bytes < numBytes; read_bytes++){
    ((byte*) (in_buffer))[read_bytes] = i2c.read();
  }
  if(in_buffer->id & 0x80){
    // Arduino messages are processed imediately
    nc.arduino_request(in_buffer);
  } else {
    // Raspberry messages are processed later
    in_message = (packet_t*) malloc(read_bytes);
    memcpy(in_message,in_buffer,read_bytes);
    in_messages.push(in_message);
  }
}

int network_controller::arduino_request(packet_t* req){
  switch(req->id){
    case ARD_ADDR: {
        // Add to network
        int *net = new int[++_net_size];
        for(int i = 0; i < _net_size-1; i++){ net[i] = _net[i]; }
        net[_net_size-1] = req->src;
        free(_net);
        _net = net;
        if(_network_state == 1){ _timeout = millis() + 100; }
        else if(_network_state == 2){ _calibration_state = 0; }
      }
      break;
    case ARD_SYNC:
      _sync_ctr++;
      break;
    default:
      break;
  }
  return 0;
}

int network_controller::raspberry_request(packet_t* req, packet_t** res){
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
        // single byte
        break;
      case 2:
        // single float
        break;
      case 3:
        // multi float
        break;
    }
  }
  return send_bytes;
}

int incoming = 0;
packet_t* received_packet, send_packet;
void network_controller::process(){
  if(Serial.available() > 0){
    incoming = Serial.read();
    switch(incoming) {
      case 49:
        _lc.setMinRef();
        break;
      case 50:
        _lc.setMaxRef();
        break;
    }
  }
  Serial.println(_lc.getIlluminance());
  if(_network_state == 0) init(); // reinit if
  // Process metrics per second
  _currentTime = millis();
  if(_currentTime-_lastTime > 1000){
    double err = _lc.getErr();
    int n = min(_ctr, 59);

    _D = _lc.getDutyCycle();
    _L = _lc.getIlluminance();
    _D_M.push(_D);
    _L_M.push(_L);

    _E += (_currentTime-_lastTime) * map(_D, 0, 255, 0, 100);
    if(_ctr>1) _CE += (_CE*(_ctr-1) + max(err,0))/_ctr;
    if(_ctr>2) _CV += (_CV*(_ctr-1) + (_L_M[n] - 2*_L_M[n-1] + _L_M[n-2])/360)/_ctr;

    if(_L_stream_state) {
      out_message = (packet_t*) _i2c.createSingleFloatPacket(RASP_ILU, _i2c.getId(), 0x69, _L);
      out_messages.push(out_message);
    }
    if(_D_stream_state) {
      out_message = (packet_t*) _i2c.createSingleFloatPacket(RASP_DUTY_CICLE, _i2c.getId(), 0x69, _D);
      out_messages.push(out_message);
    }
    _lastTime = _currentTime;
    _ctr++;
  }

  if(!in_messages.isEmpty()){
      send_bytes = 0;
      in_message = in_messages.pop();
      if(in_buffer->id & 0x80){ send_bytes = raspberry_request(in_message,&out_buffer); }
      if(send_bytes){ out_messages.push(out_buffer); }
      free(in_message);
  }

  if(!out_messages.isEmpty()){
    out_message = out_messages.pop();
    _i2c.send(out_message->dest,out_message);
    free(out_message);
  }
}

void network_controller::_sync(){
  out_message = _i2c.createPacket(ARD_SYNC, 0, _i2c.getId());
  _i2c.broadcast(out_message);
  _sync_ctr++;
  while(_sync_ctr < _net_size);
  free(out_message);
  _sync_ctr = 0;
}

void network_controller::consensus(){
  _consensus_state = 1;
}

void network_controller::calibrate(){
  _consensus_state = 0;
  _calibration_state = 1;
  int id = _i2c.getId();
  int U;

  _K = new double[_net_size];
  _O = _lc.getLight();

  _sync();
  for(int i = 0; i < _net_size; i++){
    if(i == id) _lc.setLight(255);
    delay(20);
    U = _lc.getLight();
    _K[i] = (U - _O) / 255;
    _sync();
    if(i == id) _lc.setLight(0);
  }
}

void network_controller::init(){
  _i2c.onReceive(onReceive);
  _net_size = 0;
  _network_state = 1;
  _calibration_state = 0;
  _consensus_state = 0;
  out_message = _i2c.createPacket(ARD_ADDR, 0, _i2c.getId());
  delay(5+random(10));
  _i2c.startSession(0);
  _net[_net_size] = _i2c.getId();
  _i2c.write(out_message);
  _i2c.endSession();
  free(out_message);
  _currentTime = millis();
  _timeout = _currentTime + 100;
  while(_currentTime > _timeout){ _currentTime = millis(); }
}
