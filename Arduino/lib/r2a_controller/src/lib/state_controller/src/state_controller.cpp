#include "state_controller.h"

// // Processing methods
// ProcessMetrics calculates new metrics and adds items to minute luminosity and minute dutycycle
void state_controller::_processMetrics(){
  int n = min(ctr, 59);

  // Pushes into minute buffers
  D_M.push(D);
  L_M.push(L);

  // Calculates consumed energy assuming P = 1W
  E += (currentTime-lastTime) * map(D, 0, 255, 0, 100);
  // Calculates confort error
  if(ctr>1) CE += (CE*(ctr-1) + max(err,0))/ctr;
  // Calculates confort variance
  if(ctr>2) CV += (CV*(ctr-1) + (L_M[n] - 2*L_M[n-1] + L_M[n-2])/360)/ctr;
}

void state_controller::process() {
  currentTime = millis();
  // Ensures (at least) 1 second period
  if(currentTime - lastTime > 1000){
    _processMetrics();
    // Update time
    lastTime = currentTime;
    // Update counter
    ctr++;
  }
}

void state_controller::init() {

}

// // Messaging methods
// Getters
bool state_controller::hasInMessages() { return !_in_messages.isEmpty(); }
bool state_controller::hasOutMessages() { return !_out_messages.isEmpty(); }

message_t *state_controller::getInMessage() { return _in_messages.dequeue(); }
message_t *state_controller::getOutMessage() { return _out_messages.dequeue(); }

// Setters
void state_controller::addInMessage(message_t *message) { return _in_messages.enqueue(message); }
void state_controller::addOutMessage(message_t *message) { return _out_messages.enqueue(message); }

void state_controller::addNodeToNetwork(int id){
  int *aux = new int[++net_size];
  for(int i = 0; i < net_size-1; i++){ aux[i] = net[i]; }
  aux[net_size] = id;
  free(net);
  net = aux;
  net_index = net_size;
}
