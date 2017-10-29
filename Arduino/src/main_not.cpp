#include <Arduino.h>
#include <Wire.h>
/*
const int MY_ADDRESS = 2; // Device ID
const int COMMAND_SIZE = 2; // 2 Bytes

void receiveCommand(int howMany) {
  if (howMany == COMMAND_SIZE) {
    int result;

    for(int i; i<COMMAND_SIZE; i++){
      result = Wire.read();
      result <<= 8;
      result |= Wire.read();
    }

    // Get the sender
    // Get the command
    // Get the data
  }
}

void setup() {
  Serial.begin(9600);  // start serial for output
  Wire.begin(MY_ADDRESS);      // join i2c bus (address optional for master)
  // They give us an ID so we can be slaves ^      ^           ^
  TWAR = (MY_ADDRESS << 1) | 1;  // enable broadcasts to be received
  Wire.onReceive(receiveCommand);  // set up receive handler

  // Broadcast new device event
}

void loop() {
    // put your main code here, to run repeatedly:
}
*/
