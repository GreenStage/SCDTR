#include "desk.hpp"
#include <stdint.h>
#include <time.h>

#define MESSAGE_SIZE 18


Desk::Desk(int id,int address_){
    this->address = address_;
    this->deskId = id;
}


float Desk::get_current_illuminance(){

    return lastMinuteIlluminance.back().value;
}

float Desk::get_current_duty_cicle(){
    return lastMinuteCycle.back().value;
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
