#include "datamanager.hpp"
#include <stdexcept>
#include <string>
#include <chrono>

#include <unistd.h>			
#include <fcntl.h>			
#include <sys/ioctl.h>		
#include <linux/i2c-dev.h>

using namespace std;

DataManager * DataManager::instance = NULL;
bool DataManager::initiliazed = false;

class dm_not_Initialized: public exception {

    virtual const char* what() const throw() {
      return "DataManager was not initialized.";
    }
} dmNotInitialized;
  
void DataManager::initialize(int numDesks){
    DataManager::instance = new DataManager(numDesks);
    return;
}

DataManager * DataManager::getInstance(){
    if(!DataManager::initiliazed){
        throw dmNotInitialized;
    }
    return DataManager::instance;
}

DataManager::DataManager(int numDesks, int * desk_address){
    int file_i2c;
    char *filename = (char*)"/dev/i2c-1";
    
    numDesks_ = numDesks;
    activeDesks = (Desk **) malloc(sizeof(Desk*) * numDesks_);

	if ((file_i2c = open(filename, O_RDWR)) < 0){
        throw("Failed to open the i2c bus" );
	}
    
    for(int i = 0; i < numDesks_;i++ ){ {
        try{
            activeDesks[i] = new Desk(desk_address);
        }
        catch(exception e){
            cout << "Exception creating desk " << i << " :" << e.what();
        }
    }
    
    initiliazed = true;
}

void DataManager::validate_data(){
    //TODO UPDATE EACH DESK QUEUE
    this_thread::sleep_for(std::chrono::minutes(1));
}

DataManager::~DataManager(){
    delete[] activeDesks;
}


string DataManager::parse_command(vector<string> command){

    string retval ="";
    if(command.at(0).compare("g") == 0){
        string retval = command.at(1);
        retval += " ";

        int deskIt;

        if(command.size() < 3){
            return "UNK";
        } 
        
        if(command.at(2).compare("T") == 0 ) {
            float sum = 0;
            switch(command.at(1).at(0)){
                case 'p':
                    for(int i = 0; i < ARRAYLENGTH(activeDesks); i++){
                        sum += activeDesks[i].get_power_consuption();
                    }
                    break;

                case 'e':
                    for(int i = 0; i < ARRAYLENGTH(activeDesks); i++){
                        sum += activeDesks[i].get_accumulated_energy();
                    }
                    break;
                
                case 'c':
                    for(int i = 0; i < ARRAYLENGTH(activeDesks); i++){
                        sum += activeDesks[i].get_accumulated_confort_error();
                    }
                    break;
                
                case 'v':
                    for(int i = 0; i < ARRAYLENGTH(activeDesks); i++){
                        sum += activeDesks[i].get_accumulated_confort_variance();
                    }
                    break;  

                default:
                    return "UNK";
                    break;                   
            }
            retval += "T ";
            retval += to_string(sum);
            return retval;

        } else if( ( deskIt = atoi(command.at(2).c_str() ) ) > 0){
            retval += to_string(deskIt);
            retval += " ";
            deskIt --;


            if(deskIt < 0 || deskIt > ARRAYLENGTH(activeDesks)){
                /*TODO*/
                return "UNK";
            }

            switch(command.at(1).at(0)){
                
                case 'l':
                    return retval + to_string( activeDesks[deskIt].get_current_illuminance() );

                case 'd':
                    return retval + to_string( activeDesks[deskIt].get_current_duty_cicle() );
                
                case 'o':
                    return retval + to_string( activeDesks[deskIt].get_occupancy_state() );
                
                case 'L':
                    return retval + to_string( activeDesks[deskIt].get_lower_illuminance() );
                
                case 'O':
                    return retval + to_string( activeDesks[deskIt].get_external_illuminance() );

                case 'r':
                    return retval + to_string( activeDesks[deskIt].get_illuminance_control() );
                
                case 'p':
                    return retval + to_string( activeDesks[deskIt].get_power_consuption() );
                
                case 'e':
                    return retval + to_string( activeDesks[deskIt].get_accumulated_energy() );

                case 'c':
                    return retval + to_string( activeDesks[deskIt].get_accumulated_confort_error() );
                
                case 'v':
                    return retval + to_string( activeDesks[deskIt].get_accumulated_confort_variance() );
                
                default:
                    return "UNK";
            }

        }

    }
    else if(command.at(0).compare("s") == 0){
        int deskIt;
        int val;
        if( ( deskIt = atoi(command.at(1).c_str() ) ) <= 0 || deskIt > ARRAYLENGTH(activeDesks) || ( command.at(2).compare("1") != 0 && command.at(2).compare("0") != 0 ) ){
            return "UNK";
        }
        activeDesks[--deskIt].set_occupancy_state( command.at(2).compare("1") == 0 ? true : false);

        return "ack";

    }
    else if(command.at(0).compare("r") == 0){
        restart();
        return "ack";
    }
    else if(command.at(0).compare("b") == 0){

    }

}

void DataManager::restart(){}
