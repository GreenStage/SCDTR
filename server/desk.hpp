#ifndef DESK_HEADER
#define DESK_HEADER

#include <thread>
#include <unistd.h>			
#include <fcntl.h>			
#include <sys/ioctl.h>		
#include <linux/i2c-dev.h>
#include <time.h>

#include <queue>
using namespace std;

class Desk : thread{

public:
    Desk(int,int);
    ~Desk();

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
    void set_occupancy_state(bool);

    
private:
    struct __attribute__ ((packed)) message_{
        uint8_t command,
        float val1,
        uint32_t val2
    } buffer;

    struct timed_value_{
        float value;
        time_t issued_at;
    };

    void get_data(int);
    queue<struct timed_value_> lastMinuteIlluminance;
    queue<struct timed_value_> lastMinuteCycle;
    int address;
    bool ocupancy_state;
    float lower_illuminance;
    float external_illuminance;
    float accumulated_energy;
    float accumulated_confort_error;
    float acumulated_confort_variance;
    float power_consumption;
    float control_reference;
};

#endif
