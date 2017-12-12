#include "desk.hpp"
#include <stdint.h>

#define MESSAGE_SIZE 18

using namespace std;

struct message_{
    uint8_t command,
    uint32_t val1,
    uint32_t val2
} buffer;

Desk::Desk(int fd, int address):thread(Desk::get_data,this,fd){
    address_ = address;
    if (ioctl(fd, I2C_SLAVE, address_) < 0){
        char stringErr[100];
        sprintf(stringErr,"Failed to reach slave at address: %d", address_);
        throw stringErr;
    }
}

void Desk::get_data(int fd){
	if (read(fd, &buffer, MESSAGE_SIZE) != MESSAGE_SIZE){
		printf("Failed to read from the i2c bus.\n");
	}
	else{
        cout << "Read message: " << buffer.command << ", val1 : "
             << command.val1 << ", val2: " command.val2 << endl;
	}
}

float Desk::get_current_illuminance(){
    return lastMinuteIlluminance.front();
}

float Desk::get_current_duty_cicle(){
    return lastMinuteCycle.front();
}

bool Desk::get_occupancy_state(){
    return ocupancy_state;
}

float Desk::get_lower_illuminance(){
    return lower_illuminance;
}

float Desk::get_accumulated_energy(){
    return accumulated_energy;
}

float Desk::get_accumulated_confort_error(){
    return accumulated_confort_error;
}

float Desk::get_accumulated_confort_variance(){
     return acumulated_confort_variance;
}

void Desk::set_occupancy_state(bool state){
    ocupancy_state = state;
}

float Desk::get_power_consuption(){
	return power_consumption;
}

float Desk::get_external_illuminance(){
	return external_illuminance;
}

float Desk::get_illuminance_control(){
	return control_reference;
}
