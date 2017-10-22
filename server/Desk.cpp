#include "Desk.hpp"

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