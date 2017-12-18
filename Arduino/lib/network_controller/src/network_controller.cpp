#include "network_controller.h"

extern light_controller lc;
extern i2c_controller i2c;
network_controller nc(lc, i2c);

network_controller::network_controller(light_controller& lc, i2c_controller& i2c) : _lc(lc), _i2c(i2c) {
  _dc_flag = false;
}

QueueArray<packet_t*> in_messages, out_messages;

packet_t *in_buffer, *out_buffer;
void onReceive(int numBytes){
  int read_bytes = 0;
  int send_bytes = 0;

  for(read_bytes = 0; read_bytes < numBytes; read_bytes++){
    ((byte*) (in_buffer))[read_bytes] = i2c.read();
  }

  switch(in_buffer->id & 0xC0){
    case 0x40:
      send_bytes = nc.raspberry_request(in_buffer,out_buffer);
      break;
    case 0x80:
      send_bytes = nc.arduino_request(in_buffer,out_buffer);
      break;
    case 0xC0:
      send_bytes = nc.arduino_response(in_buffer,out_buffer);
    default:
      return;
  }

  if(send_bytes){
    packet_t * out_message = (packet_t*) malloc(send_bytes);
    memcpy(out_message,out_buffer,send_bytes);
    out_messages.push(out_message);
  }
}


int network_controller::arduino_response(packet_t* req, packet_t* res){
  switch(req->id){
    default:
      break;
  }
  return 0;
}

int network_controller::arduino_request(packet_t* req, packet_t* res){
  switch(req->id){


  }
  return 0;
}

int network_controller::raspberry_request(packet_t* req, packet_t* res){
  switch(req->id){


  }
  return 0;
}

int incoming = 0;
packet_t* send_packet;
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
  if(!out_messages.isEmpty()){
      send_packet = out_messages.pop();
      _i2c.send(send_packet->dest, send_packet);
      free(send_packet);
  }
}

void network_controller::consensus(){

}

void network_controller::calibrate(){
  int *net = _i2c.getNet();
  int id = _i2c.getId();
  int U, n = size_t(net);

  _K = new double[n];
  _O = _lc.getLight();

  _i2c.sync();
  for(int i = 0; i < n; i++){
    if(i == id) _lc.setLight(255);
    delay(20);
    U = _lc.getLight();
    _K[i] = (U - _O) / 255;
    _i2c.sync();
    if(i == id) _lc.setLight(0);
  }
}

void network_controller::init(){
  _i2c.onReceive(onReceive);


}
