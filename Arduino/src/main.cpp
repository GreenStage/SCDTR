#include <Arduino.h>
#include "i2c_controller.h"
#include "state_controller.h"
#include "light_controller.h"
#include "r2a_controller.h"
#include "a2a_controller.h"

i2c_controller i2c;
state_controller state;
light_controller lc(state);
r2a_controller r2a(state, i2c);
a2a_controller a2a(state, i2c, lc);

volatile unsigned long currentTime;
volatile unsigned long lastTime = micros();
void lightInterrupt(){
  currentTime = micros();
  if(currentTime-lastTime > 40000){
    lc.process();
    lastTime = currentTime;
  }
}

void messageInterupt(int numBytes){
  message_t *message = i2c.read(numBytes);
  if((message->id & 0xC0) == 0x80) a2a.handleReceive(message);
  else if((message->id & 0xC0) == 0x40) r2a.handleReceive(message);
}

void setup() {
  Serial.begin(9600);
  state.init();
  lc.init(lightInterrupt);
  i2c.init(messageInterupt);
}

void loop() {
  if(state.hasInMessages()){
    message_t *message = state.getInMessage();
    if((message->id & 0xC0) == 0x40) r2a.handleResponse(message);
    free(message);
  }

  a2a.process();
  state.process();
  r2a.process();

  if(state.hasOutMessages()){
    message_t *message = state.getOutMessage();
    i2c.send(message->dest, message);
    free(message);
  }
}
