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
    case ARD_CONSENSUS: {
      multiple_float_packet *p = (multiple_float_packet*) req;
      updateDcCopy(p->val, p->n_data);
    }
    case ARD_ADDR: {
        // Add to network
        _insert(req->src);
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

void network_controller::updateDcCopy(float data[MAX_LEN], int len){
  for(int i = 0; i < len; i++) _dc_copy[i] = data[i];
  _dc_flag=true;
}

multiple_float_packet* message;
void network_controller::consensus(){
  _consensus_state = 1;
  float k11;
  float k12;
  float d2_copy[2];
  float b_y1[2];
  int i;
  float a;
  int b_i;
  float d11_best, d12_best;
  float min_best_1;
  int sol_unconstrained;
  int sol_boundary_linear;
  int sol_boundary_0;
  int sol_boundary_100;
  int sol_linear_0;
  int sol_linear_100;
  float z11, z12;
  float u1;
  float n;
  float w1, w2, w3;
  float d11u, d12u;
  float b_a;
  float d11bl;
  float min_unconstrained;
  float d12bl, d12b0;
  float min_boundary_linear;
  float min_boundary_0;
  float d12b100;
  float min_boundary_100;
  float common;
  float det2, det4;
  float x1, v1;
  float d11l0, d12l0;
  float min_linear_0;
  float d11l100, d12l100;
  float d[2];
  float d1[2];
  float b_d_;
  float R = _lc.getRef();
  if(_net_index==1){
    k11 = _K[0];
    k12 = _K[1];
  }else if(_net_index==2){
    k11 = _K[1];
    k12 = _K[0];
  }

  for (i = 0; i < 2; i++) {
    d[i] = 0.0;
    d2_copy[i] = 0.0;
    b_y1[i] = 0.0;
  }

  /* iterations */
  for (i = 0; i < 50; i++) {
    /*  node 1 */
    d11_best = -1.0;
    d12_best = -1.0;
    min_best_1 = 100000.0;

    /* big number */
    sol_unconstrained = 1;
    sol_boundary_linear = 1;
    sol_boundary_0 = 1;
    sol_boundary_100 = 1;
    sol_linear_0 = 1;
    sol_linear_100 = 1;
    z11 = (-1.0 - b_y1[0]) + 0.01 * d[0];
    z12 = -b_y1[1] + 0.01 * d[1];
    u1 = _O - R;
    n = k11 * k11 * 100.0 + k12 * k12 * 100.0;
    w1 = -k11 * 100.0 * z11 - k12 * z12 * 100.0;
    w2 = -z11 * 100.0;
    w3 = z11 * 100.0;

    /* compute unconstrained minimum */
    d11u = 100.0 * z11;
    d12u = 100.0 * z12;

    /* check feasibility of unconstrained minimum using local constraints */
    if (d11u < 0.0) { sol_unconstrained = 0; }
    if (d11u > 100.0) { sol_unconstrained = 0; }
    if (k11 * d11u + k12 * d12u < R - _O) { sol_unconstrained = 0; }

    /*  compute function value and if best store new optimum */
    if (sol_unconstrained != 0) {
      b_a = d11u - d[0];
      a = d12u - d[1];
      min_unconstrained = ((((0.0 * (d11u * d11u) + d11u) + b_y1[0] * (d11u - d[0]))
		    + b_y1[1] * (d12u - d[1])) + 0.005 * (b_a * b_a)) + 0.005 * (a * a);
      if (min_unconstrained < 1000) {
        d11_best = d11u;
        d12_best = d12u;
        min_best_1 = min_unconstrained;
      }
    }

    /* compute minimum constrained to linear boundary    */
    d11bl = 100.0 * z11 + 100.0 * k11 / n * (w1 - u1);
    d12bl = 100.0 * z12 + 100.0 * k12 / n * (w1 - u1);

    /* check feasibility of minimum constrained to linear boundary */
    if (d11bl < 0.0) { sol_boundary_linear = 0; }
    if (d11bl > 100.0) { sol_boundary_linear = 0; }

    /*  compute function value and if best store new optimum */
    if (sol_boundary_linear != 0) {
      b_a = d11bl - d[0];
      a = d12bl - d[1];
      min_boundary_linear = ((((0.0 * (d11bl * d11bl) + d11bl) + b_y1[0] * d11bl - d[0])) + b_y1[1] * (d12bl - d[1])) + 0.005 * (b_a * b_a) + 0.005 * (a * a);
      if (min_boundary_linear < min_best_1) {
        d11_best = d11bl;
        d12_best = d12bl;
        min_best_1 = min_boundary_linear;
      }
    }

    /* compute minimum constrained to 0 boundary */
    d12b0 = 100.0 * z12;

    /* check feasibility of minimum constrained to 0 boundary */
    if (k11 * 0.0 + k12 * d12b0 < R - _O) { sol_boundary_0 = 0; }

    /*  compute function value and if best store new optimum */
    if (sol_boundary_0 != 0) {
      b_a = d12b0 - d[1];
      min_boundary_0 = ((b_y1[0] * (0.0 - d[0]) + b_y1[1] * (d12b0 - d[1])) + 0.005
		    * ((0.0 - d[0]) * (0.0 - d[0]))) + 0.005 * (b_a * b_a);
      if (min_boundary_0 < min_best_1) {
        d11_best = 0.0;
        d12_best = d12b0;
        min_best_1 = min_boundary_0;
      }
    }

    /* compute minimum constrained to 100 boundary */
    d12b100 = 100.0 * z12;
    /* check feasibility of minimum constrained to 100 boundary */
    if (k11 * 100.0 + k12 * d12b100 < R - _O) { sol_boundary_100 = 0; }
    /*  compute function value and if best store new optimum */
    if (sol_boundary_100 != 0) {
      b_a = d12b100 - d[1];
      min_boundary_100 = (((100.0 + b_y1[0] * (100.0 - d[0])) + b_y1[1] *(d12b100 - d[1]))
        + 0.005 * ((100.0 - d[0]) * (100.0 - d[0]))) + 0.005 * (b_a * b_a);
      if (min_boundary_100 < min_best_1) {
        d11_best = 100.0;
        d12_best = d12b100;
        min_best_1 = min_boundary_100;
      }
    }

    /*  compute minimum constrained to linear and zero boundary */
    common = 0.01 / (0.01 * n - k11 * k11);
    det2 = -k11 * common;
    det4 = n * 0.01 * common;
    x1 = common * w1 + det2 * w2;
    v1 = common * u1 + det2 * 0.0;

    /* u2 = 0 so this can be simplified */
    /* u2 = 0 so this can be simplified */
    d11l0 = (100.0 * z11 + 100.0 * k11 * (x1 - v1)) + 100.0
      * ((det2 * w1 + det4 * w2) - (det2 * u1 + det4 * 0.0));
    d12l0 = 100.0 * z12 + 100.0 * k12 * (x1 - v1);

    /* check feasibility */
    if (d11l0 > 100.0) { sol_linear_0 = 0; }

    /*  compute function value and if best store new optimum */
    if (sol_linear_0 != 0) {
      b_a = d11l0 - d[0];
      a = d12l0 - d[1];
      min_linear_0 = ((((0.0 * (d11l0 * d11l0) + d11l0) + b_y1[0] * (d11l0 - d[0]))
        + b_y1[1] * (d12l0 - d[1])) + 0.005 * (b_a * b_a)) + 0.005 * (a * a);
      if (min_linear_0 < min_best_1) {
        d11_best = d11l0;
        d12_best = d12l0;
        min_best_1 = min_linear_0;
      }
    }

    /*  compute minimum constrained to linear and 100 boundary */
    det2 = k11 * common;
    det4 = n * 0.01 * common;
    x1 = common * w1 + det2 * w3;
    v1 = common * u1 + det2 * 100.0;
    d11l100 = (100.0 * z11 + 100.0 * k11 * (x1 - v1)) - 100.0
      * ((det2 * w1 + det4 * w3) - (det2 * u1 + det4 * 100.0));
    d12l100 = 100.0 * z12 + 100.0 * k12 * (x1 - v1);

    /* check feasibility */
    if (d11l100 < 0.0) { sol_linear_100 = 0; }

    /*  compute function value and if best store new optimum */
    if (sol_linear_100 != 0) {
      b_a = d11l100 - d[0];
      a = d12l100 - d[1];
      if (((((0.0 * (d11l100 * d11l100) + d11l100) + b_y1[0] * (d11l100 - d[0]))
          + b_y1[1] * (d12l100 - d[1])) + 0.005 * (b_a * b_a)) + 0.005 * (a * a) < min_best_1) {
        d11_best = d11l100;
        d12_best = d12l100;
      }
    }

    /* store data and save for next cycle */
    d1[0] = d11_best;
    d1[1] = d12_best;

    /* compute average with available knowledge */
    /* update local lagrangian */
    /* Recebe do outro arduino */
    message = _i2c.createMultiFloatPacket(ARD_CONSENSUS, _i2c.getId(), _net[b_i], _net_size);
    for(b_i=0; b_i<_net_size-1; b_i++){ message->val[b_i] = d1[b_i]; }

    _i2c.startSession(message->dest);
    _i2c.write((packet_t*) message, sizeof(packet_t)+1+sizeof(float)*(b_i));
    _i2c.endSession();

    while(_dc_flag == false);
    _dc_flag = false;

    for(b_i=0; b_i<_net_size-1; b_i++){ d2_copy[b_i] = _dc_copy[_net_size-1-i]; }

    for (b_i = 0; b_i < 2; b_i++) {
      b_d_ = (d1[b_i] + d2_copy[b_i]) / 2.0;
      d2_copy[b_i]++;
      d[b_i] = b_d_;
      b_y1[b_i] += 0.01 * (d1[b_i] - b_d_);
    }
  }

  a = 0.0;
  for (b_i = 0; b_i < 2; b_i++) { a += _K[b_i] * d[b_i]; }

  _R = a + _O;
  _lc.setRef(_R);
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
  _calibration_state = 2;
}

void network_controller::_insert(int id){
  int *net = new int[++_net_size];
  for(int i = 0; i < _net_size-1; i++){ net[i] = _net[i]; }
  net[_net_size] = id;
  free(_net);
  _net = net;
  _net_index = _net_size;
}

void network_controller::init(){
  _i2c.onReceive(onReceive);
  _net_size = 0;
  _network_state = 1;
  _calibration_state = 0;
  _consensus_state = 0;
  out_message = _i2c.createPacket(ARD_ADDR, 0, _i2c.getId());
  delay(10*random(10));
  _i2c.startSession(0);
  _insert(_i2c.getId());
  _i2c.write(out_message, _i2c.sizeOfPacket(ARD_ADDR));
  _i2c.endSession();
  free(out_message);
  _currentTime = millis();
  _timeout = _currentTime + 100;
  while(_currentTime > _timeout){ _currentTime = millis(); }
}
