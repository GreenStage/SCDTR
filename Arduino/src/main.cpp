#include <Arduino.h>
#include "i2c_controller.h"
#include "state_controller.h"
#include "light_controller.h"
#include "r2a_controller.h"
#include "a2a_controller.h"

// Instanciate controllers
// I2C for comunications with i2c bus
i2c_controller i2c;
// State controller for centralizing the state information
state_controller state;
// Light controller to implement the feedback and solo feedforward
light_controller lc(state);
// Raspberry to Arduino controller to process the comunications with the rpi
r2a_controller r2a(state, i2c);
// Arduino to Arduino controller to implement distributed calibration and consensus
a2a_controller a2a(state, i2c, lc);

// Prepare light controller interruption
volatile unsigned long currentTime;
volatile unsigned long lastTime = micros();
void lightInterrupt(){
  // Update current time
  currentTime = micros();
  // Check if a period has passed since the last iteration
  if(currentTime-lastTime > 40000){
    // Process light controller
    lc.process();
    // Update iteration time
    lastTime = currentTime;
  }
}

// I2C Async message receiving
void messageInterupt(int numBytes){
  message_t *message = i2c.read(numBytes);
  // Process arduino messages
  if((message->id & 0xC0) == 0x80) a2a.handleReceive(message);
  // Process raspberry messages
  else if((message->id & 0xC0) == 0x40) r2a.handleReceive(message);
}

void setup() {
  Serial.begin(9600);
  // Initialize state with default variables
  state.init();
  // Initialize light controller with interruption
  lc.init(lightInterrupt);
  // Initialize i2c controller with message received callback
  i2c.init(messageInterupt);
}

void loop() {
  // Check for received messages
  if(state.hasInMessages()){
    // Pop a message from receiving queue
    message_t *message = state.getInMessage();
    // Process response
    if((message->id & 0xC0) == 0x40) r2a.handleResponse(message);
    // Release message's memory space
    free(message);
  }

  // Process
  a2a.process();
  // Update metrics
  state.process();
  // Update streams
  r2a.process();

  // Check for sending messages
  if(state.hasOutMessages()){
    // Pop message from sending queue
    message_t *message = state.getOutMessage();
    // Send message
    i2c.send(message->dest, message);
    // Release message's memory space
    free(message);
  }
}
