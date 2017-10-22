#include <stack>

class Desk{

public:
    float get_current_illuminance();
    float get_current_duty_cicle();
    bool get_occupancy_state();
    float get_lower_illuminance();
    float get_external_illuminance();
    float get_illuminance_control();
    float get_power_consuption();

    float get_accumulated_energy();

    float get_accumulated_confort_error();
   
    float get_accumulated_confort_variance();
    bool set_occupancy_state();

    
private:
    std::stack<float> lastMinuteIlluminance;
    std::stack<float> lastMinuteCycle;

};