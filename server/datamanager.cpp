#include "datamanager.hpp"
#include <stdexcept>
#include <string>
#include <chrono>

#include <unistd.h>			
#include <fcntl.h>			
#include <sys/ioctl.h>		
#include <linux/i2c-dev.h>
#include <wiringSerial.h>

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
        throw "Failed to open the i2c bus";
	}
    
    if( (serialFd == serialOpen ("/dev/ttyUSB0", 115200 )) ){
        throw "Failed to open serial fd";
    }
    
    for(int i = 0; i < numDesks_;i++ ){ {
        try{
            activeDesks[i] = new Desk(desk_address);
        }
        catch(exception e){
            cout << "Exception creating desk " << i << " :" << e.what();
        }
    } 
    
    this.exit = false;
        rqsts_it = pending_requests.begin();
        resps_it = pending_responses.begin();

    thread i2cthread = thread(DataManager::parseResponse,this);
    initiliazed = true;
}


void DataManager::request(int deskId, int type){
    request_to_arduino * rqst = NULL;

    int i,addr = -1;
 

    for(i = 0; i < numDesks_;i++){
        if(activeDesks[i] == deskId){
            rqst = (request*) calloc(0,sizeof(request_to_arduino));
            rqst->address = addr = activeDesks[i].address;
            rqst->type = type;
            break;
        }
    }
    
    if(rqst == NULL) continue;

    if (ioctl(file_i2c, I2C_SLAVE, addr) < 0){
        char stringErr[100];
        sprintf(stringErr,"Failed to reach slave at address: %d", addr);
        cout << stringErr << endl;
        free(rqst);
        return;
    }

    if (write(file_i2c, rqst, sizeof(request_to_arduino)) != sizeof(request_to_arduino)) {
        cout << "Failed to write to the i2c bus.\n" << endl;
        free(rqst);
        return;
    }

    pending_requests.push_back(rqst);

    string response = fetch_response(deskId,addr,type);
}

string DataManager::fetch_response(int deskid,int deskAddr,int type){
    int timeoutCtr = 0;
    while(pending_responses.empty()){
        if(timeoutCtr > 5){
            printf("Timeout waiting for response from arduino 15seconds given\n");
            return;
        }
        timeoutCtr++;
        this_thread::sleep_for(std::chrono::seconds(1));
    }    
    for(resps_it = pending_responses.begin(); resps_it != arduino_default_resp.end(); ++resps_it){
        if(resps_it->address == deskAddr && resps_it->type == type){
#ifdef DEBUG
            printf("Receive response from dm %d, address: %d, type: %d",deskid,,deskAddr,type);
#endif
            /*TODO: parse message type, and respond to client*/       

            return "oi";
        }
    }
}
void DataManager::parseResponse(){
    arduino_default_resp * default_in_buff;
    int readBytes;
    while(!this.exit){
        while(pending_requests.empty()){
            this_thread::sleep_for(std::chrono::seconds(1));
        }

        default_in_buff = ( arduino_default_resp*) malloc(100);

        if (ioctl(file_i2c, I2C_SLAVE, addr) < 0){
            char stringErr[100];
            sprintf(stringErr,"Failed to reach slave at address: %d", addr);
            cout << stringErr << endl;
            free(rqst);
            return;
        }

        if ( (readBytes = read(file_i2c, default_in_buff, 100)) < sizeof(arduino_default_resp)){
            cout << "Failed to read from the i2c bus.\n" << endl;
        }

        for(rqsts_it = pending_requests.begin(); rqsts_it != pending_requests.end(); ++rqsts_it){
#ifdef DEBUG
            printf("Response received from %d , type : %d\n",default_in_buff->address,default_in_buff->type);
#endif
            if(rqsts_it->address == default_in_buff->address){
                request_to_arduino * rqst = &*rqsts_it;
                pending_requests.erase(rqsts_it);
                rqsts_it = pending_requests.begin();
                pending_responses.push_back(default_in_buff);
                free(rqst);
                break;
            }
        }
    }
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
