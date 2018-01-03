#include "a2a_controller.h"

a2a_controller::a2a_controller(state_controller& state, i2c_controller& i2c, light_controller& lc) : _state(state), _i2c(i2c), _lc(lc) {}

void a2a_controller::process(){
    if(_state.network_state == 0) initNetwork();
    if(_state.calibration_state == 0) calibrate();
    if(_state.consensus_state == 0) consensus();
}

void a2a_controller::_sync(){
  message_t *message = _i2c.simpleMessage(ARD_SYNC, _i2c.getId(), 0);
  _state.sync_ctr++;
  _i2c.broadcast(message);
  while(_state.calibration_state == 1 && _state.sync_ctr < _state.net_size);
  _state.sync_ctr = 0;
  free(message);
}

void a2a_controller::handleConsensus(multi_float_message_t *message) {
  // actualiza a cópia dos valores do vizinho
  for(int i=0; _state.net_size-1; i++) { _state.dc_copy[i] = message->data[i]; }
  _state.dc_flag = true;
}

void a2a_controller::handleNetHello(message_t *message) {
  if(_state.network_state == 2) {
    _state.sync_ctr = 0;
    _state.net_size = 0;
    _state.network_state = 0;
    _state.consensus_state = 0;
    _state.calibration_state = 0;
  }
}

void a2a_controller::handleNetAddr(message_t *message) {
  if(_state.network_state != 2) {
    _state.addNodeToNetwork(message->src);
    if(_state.network_state == 1) { _state.timeout = millis() + 100; }
  }
}

void a2a_controller::handleReceive(message_t *message) {
  switch(message->id){
    case ARD_CONSENSUS:
      Serial.println("Recebi mesmo um consensus");
      handleConsensus((multi_float_message_t *) message);
      break;
    case ARD_HELLO:
      handleNetHello(message);
      break;
    case ARD_ADDR:
      handleNetAddr(message);
      break;
    case ARD_SYNC:
      _state.sync_ctr++;
      break;
    default:
      break;
  }
}

void a2a_controller::initNetwork() {
  message_t *message;

  // Get my id
  int id = _i2c.getId();

  // Update state
  _state.network_state = 1;

  // Create hello message
  message = _i2c.simpleMessage(ARD_HELLO, id, 0);

  // Broadcast hello message
  _i2c.broadcast(message);

  // Release message memory space
  free(message);

  // Create address message
  message = _i2c.simpleMessage(ARD_ADDR, id, 0);

  // Wait to make sure others have updated their state after receiving the hello message
  delay(100+10*random(10));

  // Start I2C session to maintain atomicity
  _i2c.startSession(0);

  // Add own id to the network
  _state.addNodeToNetwork(id);

  // Update own network index
  _state.net_index = _state.net_size-1;

  // Send address to all arduinos
  _i2c.write(message, _i2c.messageSize(_i2c.messageType(ARD_ADDR)));

  // End I2C session
  _i2c.endSession();

  // Release message memory space
  free(message);

  // Prepare timeout
  _state.currentTime = millis();
  // Wait a sec
  _state.timeout = _state.currentTime + 100;

  // Wait for the rest
  while(_state.currentTime < _state.timeout){ _state.currentTime = millis(); }

  // Update state
  _state.network_state = 2;
  printNet();
}

void a2a_controller::printNet(){
  Serial.print("Index: ");
  Serial.println(_state.net_index);
  Serial.println("Rede:");
  for(int i=0; i<_state.net_size; i++){
    Serial.println(_state.net[i]);
  }
}

void a2a_controller::printCalib(){
  Serial.println("My offset lighting is:");
  Serial.print(_state.O);
  Serial.println(" lux");
  Serial.println("My max lighting is:");
  Serial.print(_state.M);
  Serial.println(" lux");
  Serial.println("My calibration K's are:");
  for(int i=0; i<_state.net_size; i++){
    Serial.println(_state.K[i]);
    Serial.println(" lux");
  }
}

void a2a_controller::calibrate() {
  float M;
  // Update state
  _state.calibration_state = 1;

  // Prepare K gains array
  _state.K = new float[_state.net_size];

  // Turn off the light
  _lc.setLight(0);

  // Wait a period
  delay(_state.T);

  // Get ambient light
  _state.O = _lc.getLight();

  // Iterate for each arduino in the  network
  for(int i = 0; i < _state.net_size; i++){

    // If it's my turn, turn the light on
    if(i == _state.net_index) _lc.setLight(255);

    // Wait a period
    delay(_state.T);

    // Read illumination
    M = _lc.getLight();

    // Compute gain
    _state.K[i] = (M - _state.O) / 255;

    // If it's my turn, update state max illumination
    if(i == _state.net_index) _state.M = M;

    // If it's my turn, turn the light off
    if(i == _state.net_index) _lc.setLight(0);
  }

  // Compute my feedforward gain
  _state.Kff = _state.K[_state.net_index];

  _state.R = _state.ocupancy ? 2 * _state.M / 3 : _state.M / 3;

  // Update state
  _state.calibration_state = 2;
  printCalib();
}

void a2a_controller::printConse(float *d){
  Serial.println("Reached a Consensus with these values:");
  for(int i=0; i<_state.net_size; i++){
    Serial.print(d[i]);
    Serial.print(" duty cicle for Ard nº ");
    Serial.println(i);
  }
}

void a2a_controller::consensus(){
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

  Serial.println("ENTREI NO CONSENSUS");
  // Update state
  _state.consensus_state = 1;

  if(_state.net_index==0){
    k11 = _state.K[0]*2.55;
    k12 = _state.K[1]*2.55;
  } else if(_state.net_index==1){
    k11 = _state.K[1];
    k12 = _state.K[0];
  }

  for (i=0; i<2; i++) {
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
    u1 = _state.O - _state.L;
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
    if (k11 * d11u + k12 * d12u < _state.R - _state.O) { sol_unconstrained = 0; }

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
    if (k11 * 0.0 + k12 * d12b0 < _state.R - _state.O) { sol_boundary_0 = 0; }

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
    if (k11 * 100.0 + k12 * d12b100 < _state.R - _state.O) { sol_boundary_100 = 0; }
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
    Serial.print("A MINHA ITERAÇÃO:");
    Serial.println(d1[0]);
    Serial.println(d1[1]);

    d[0]=d[0]*255;
    d[1]=d[1]*255;

    multi_float_message_t *message = _i2c.multiFloatMessage(ARD_CONSENSUS, _i2c.getId(), _state.net[b_i], _state.net_size);
    for(b_i=0; b_i<_state.net_size-1; b_i++){ message->data[b_i] = d1[b_i]; }
    _i2c.send(message->dest, (message_t*) message);

    Serial.println("ENVIEI A ITERAÇÃO");
    while(_state.dc_flag == false && _state.consensus_state == 1);
    _state.dc_flag = false;
    Serial.println("Acho que recebi consensus");
    for(b_i=0; b_i<_state.net_size-1; b_i++){ d2_copy[b_i] = _state.dc_copy[_state.net_size-1-i]; }

    for (b_i = 0; b_i < 2; b_i++) {
      b_d_ = (d1[b_i] + d2_copy[b_i]) / 2.0;
      d[b_i] = b_d_;
      b_y1[b_i] += 0.01 * (d1[b_i] - b_d_);
    }
  }

  a = 0.0;
  for (b_i = 0; b_i < 2; b_i++) { a += _state.K[b_i] * d[b_i]; }

  _state.R = a + _state.O;

  printConse(d);
}
