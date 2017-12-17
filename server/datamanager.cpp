#include "datamanager.hpp"
#include <stdexcept>
#include <string>
#include <chrono>

#include <string.h>
#include <unistd.h>			
#include <fcntl.h>			
#include <sys/ioctl.h>		
#include <linux/i2c-dev.h>
#include <wiringSerial.h>
#include <pigpio.h>

#define RPI_SLAVE_ADDR 0x48
using namespace std;

enum packet_ids{
  PACKET_NONE = 0,
  RASP_RQST_MIN = 0x40,
  RASP_RQST_ILU,
  RASP_RQST_DUTY_CICLE,
  RASP_RQST_LOWER_ILUMINANCE,
  RASP_RQST_ACC_ENERGY,
  RASP_RQST_ACC_CONFORT_ERR,
  RASP_RQST_ACC_CONFORT_VAR,
  RASP_RQST_POW_CONSUP,
  RASP_RQST_EXT_ILU,
  RASP_RQST_ILU_CTR,
  RASP_RQST_OCCUPANCY_ST,
  RASP_RQST_SET_NOT_OCCUP,
  RASP_RQST_SET_OCCUP,
  RASP_RQST_MAX,
  
  ARD_RQST_MIN = 0x80,
  ARD_RQST_REF,
  //ARDUINO_TO_ARDUINO REQUEST 3PACKETS
  ARD_RQST_MAX,

  ARD_RESP_MIN = 0xC0,
  ARD_RESP_ILU,
  ARD_RESP_DUTY_CICLE,
  ARD_RESP_LOWER_ILUMINANCE,
  ARD_RESP_ACC_ENERGY,
  ARD_RESP_ACC_CONFORT_ERR,
  ARD_RESP_ACC_CONFORT_VAR,
  ARD_RESP_POW_CONSUP,
  ARD_RESP_EXT_ILU,
  ARD_RESP_ILU_CTR,
  ARD_RESP_OCCUPANCY_ST,
  ARD_RESP_MAX
};

int response_of(int id){
  switch(id){
    case RASP_RQST_ILU:               return ARD_RESP_ILU;
    case RASP_RQST_DUTY_CICLE:        return ARD_RESP_DUTY_CICLE;
    case RASP_RQST_LOWER_ILUMINANCE:  return ARD_RESP_LOWER_ILUMINANCE;
    case RASP_RQST_ACC_ENERGY:        return ARD_RESP_ACC_ENERGY;
    case RASP_RQST_ACC_CONFORT_ERR:   return ARD_RESP_ACC_CONFORT_ERR;
    case RASP_RQST_ACC_CONFORT_VAR:   return ARD_RESP_ACC_CONFORT_VAR;
    case RASP_RQST_POW_CONSUP:        return ARD_RESP_POW_CONSUP;
    case RASP_RQST_EXT_ILU:           return ARD_RESP_EXT_ILU;
    case RASP_RQST_ILU_CTR:           return ARD_RESP_ILU_CTR;
    case RASP_RQST_OCCUPANCY_ST:      return ARD_RESP_OCCUPANCY_ST;
	default:						  return PACKET_NONE;    
  }
}

DataManager * DataManager::instance = NULL;
bool DataManager::initiliazed = false;

class dm_not_Initialized: public exception {

    virtual const char* what() const throw() {
      return "DataManager was not initialized.";
    }
} dmNotInitialized;
  
void DataManager::initialize(int numDesks,int * desk_address){
    DataManager::instance = new DataManager(numDesks,desk_address);

    return;
}

DataManager * DataManager::getInstance(){
    if(!DataManager::initiliazed){
        throw dmNotInitialized;
    }
    
    return DataManager::instance;
}

DataManager::DataManager(int numDesks, int * desk_address){
    char *filename = (char*)"/dev/i2c-1";
      
    numDesks_ = numDesks;
    activeDesks = (Desk **) malloc(sizeof(Desk*) * numDesks_);
    
	if( (serialFd == serialOpen("/dev/ttyUSB0", 9600)) ){
        throw "Failed to open serial fd";
    }
    
    for(int i = 0; i < numDesks_;i++ ){ 
        try{
            activeDesks[i] = new Desk(i,desk_address[i]);
        }
        catch(exception e){
            cout << "Exception creating desk " << i << " :" << e.what() << endl;
        }
    } 
    
	
    this->exit = false;
	rqsts_it = pending_requests.begin();
	resps_it = pending_responses.begin();

    thread i2cthread = thread(&DataManager::parseResponse,this);

    initiliazed = true;
	i2cthread.detach();
}


string DataManager::request(int deskId, int type){

    request_to_arduino rqst;

    int ret,i,addr = -1;
    int dataCtr = 0;
    
    for(i = 0; i < numDesks_;i++){
        if(activeDesks[i]->deskId == deskId){
            addr = activeDesks[i]->address;
            rqst.src_address = RPI_SLAVE_ADDR;
            rqst.dest_address = activeDesks[i]->address;
            rqst.type = type;
            break;
        }
    }
    
    if(i == numDesks_) return "Desk specified does not exist";
   /* 
    if(sizeof(request_to_arduino) != write(serialFd,&rqst,sizeof(request_to_arduino))){
        cout << "Failed to write to serial."  << endl;
        return "Failed to write to serial\n";
    }
    else{
			printf("Sent packet:\t%d\n",rqst.type);
			printf("\tto address: %d\n", rqst.dest_address);
	}*/

/* Initialize Master*/
	printf("Sent %d %d %d\n",rqst.src_address ,rqst.dest_address ,rqst.type);
	file_i2c = i2cOpen(1, rqst.dest_address, 0);
	
	if( (ret = i2cWriteDevice(file_i2c, (char *) &rqst, sizeof(request_to_arduino))))
		printf("error writing to i2c %d\n",ret);
	
	i2cClose(file_i2c);
	
	listeningTo = addr;
    pending_requests.push_back(rqst);

    return fetch_response(deskId,addr, response_of(type) );
}

string DataManager::fetch_response(int deskid,int deskAddr,int type){
    int timeoutCtr = 0;
    while(pending_responses.empty()){
        if(timeoutCtr > 15){
            printf("Timeout waiting for response from arduino 15seconds given\n");
            return "";
        }
        timeoutCtr++;
        this_thread::sleep_for(std::chrono::seconds(1));
    }    
    for(resps_it = pending_responses.begin(); resps_it != pending_responses.end(); ++resps_it){
        if(resps_it->src_address == deskAddr && resps_it->type == type){
#ifdef DEBUG
            printf("Processed response from dm %d, address: %d, type: %d",deskid,deskAddr,type);
#endif
			pending_responses.erase(resps_it);
            return string.to_string(resps_it->);
        }
    }
    return "nack";
}
void DataManager::parseResponse(){
	int r;
    arduino_default_resp * default_in_buff = ( arduino_default_resp*) malloc(100);
    int readBytes,status;
    
    if (gpioInitialise() < 0) {
		cout << "Error initializing gpio" <<endl;
		return;
	}
	
	/* Initialize Slave*/
	gpioSetMode(18 ,PI_ALT3);
	gpioSetMode(19, PI_ALT3);

	bsc_xfer_t xfer;	
	memset(&xfer,0,sizeof(bsc_xfer_t));
	xfer.control = (RPI_SLAVE_ADDR<<16) | (0x00<<13) | /* invert transmit status flags */
				   (0x00<<12) | /* enable host control */
				   (0x00<<11) | /* enable test fifo */
				   (0x00<<10) | /* inverte receive status flags */
				   (0x01<<9) | /* enable receive */
				   (0x01<<8) | /* enable transmit */
				   (0x00<<7) | /* abort operation and clear FIFOs */
				   (0x00<<6) | /* send control register as first I2C byte */
				   (0x00<<5) | /* send status register as first I2C byte */
				   (0x00<<4) | /* set SPI polarity high */
				   (0x00<<3) | /* set SPI phase high */
				   (0x01<<2) | /* enable I2C mode */
				   (0x00<<1) | /* enable SPI mode */
				   0x01 ;      /* enable BSC peripheral */

    while(!this->exit){
        while(pending_requests.empty()){
            this_thread::sleep_for(std::chrono::milliseconds(300));
        }        
		while(xfer.rxCnt == 0){
				printf("waiting..\n");
				status = bscXfer(&xfer);
				this_thread::sleep_for(std::chrono::milliseconds(300));
		}
	

		printf("Got from arduino : %d bytes, status %d\n", xfer.rxCnt,status);
		memcpy(default_in_buff, xfer.rxBuf,xfer.rxCnt);
		printf("Got from arduino packed id: %d\n",default_in_buff->type);
		            this_thread::sleep_for(std::chrono::milliseconds(2000));
        for(rqsts_it = pending_requests.begin(); rqsts_it != pending_requests.end(); ++rqsts_it){
            if(rqsts_it->dest_address == default_in_buff->src_address){
#ifdef DEBUG
            printf("Response received from %d , type : %d\n",default_in_buff->address,default_in_buff->type);
#endif
                pending_responses.push_back(*default_in_buff);
				pending_requests.erase(rqsts_it);
                break;
            }
        }
    }

	xfer.control = 0;
	status = bscXfer(&xfer);
    if (status >= 0) {
        printf("%d\n", status);
	}
	gpioTerminate();
}

DataManager::~DataManager(){
    delete[] activeDesks;
}


string DataManager::parse_command(vector<string> command){

    string retval ="";
    if(command.at(0).compare("g") == 0){
		if(command.size() < 3){
            return "UNK0";
        } 
        string retval = command.at(1);
        retval += " ";

        int deskIt;


        
        if(command.at(2).compare("T") == 0 ) {
            float sum = 0;
            switch(command.at(1).at(0)){
                case 'p':
                    for(int i = 0; i < ARRAYLENGTH(activeDesks); i++){
                        sum += activeDesks[i]->get_power_consuption();
                    }
                    break;

                case 'e':
                    for(int i = 0; i < ARRAYLENGTH(activeDesks); i++){
                        sum += activeDesks[i]->get_accumulated_energy();
                    }
                    break;
                
                case 'c':
                    for(int i = 0; i < ARRAYLENGTH(activeDesks); i++){
                        sum += activeDesks[i]->get_accumulated_confort_error();
                    }
                    break;
                
                case 'v':
                    for(int i = 0; i < ARRAYLENGTH(activeDesks); i++){
                        sum += activeDesks[i]->get_accumulated_confort_variance();
                    }
                    break;  

                default:
                    return "UNK2";
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
                return "UNK3";
            }

            switch(command.at(1).at(0)){
                
                case 'l':
                    return request(deskIt,RASP_RQST_ILU);

                case 'd':
					return request(deskIt,RASP_RQST_DUTY_CICLE);
                
                case 'o':
                    return request(deskIt,RASP_RQST_OCCUPANCY_ST);
                
                case 'L':
                    return request(deskIt,RASP_RQST_LOWER_ILUMINANCE);
                
                case 'O':
                    return request(deskIt,RASP_RQST_EXT_ILU);

                case 'r':
                    return request(deskIt,RASP_RQST_ILU_CTR);
                
                case 'p':
                    return request(deskIt,RASP_RQST_LOWER_ILUMINANCE);
                
                case 'e':
                    return request(deskIt,RASP_RQST_ACC_ENERGY);

                case 'c':
                    return request(deskIt,RASP_RQST_ACC_CONFORT_ERR);
                
                case 'v':
                    return request(deskIt,RASP_RQST_ACC_CONFORT_VAR);
                
                default:
                    return "UNK 5";
            }

        }
		return "UNK10";
    }
    else if(command.at(0).compare("s") == 0){
        int deskIt;
        int val;
        if(command.size() < 3){
            return "UNK 6";
        } 
        if( ( deskIt = atoi(command.at(1).c_str() ) ) <= 0 || deskIt > ARRAYLENGTH(activeDesks) || ( command.at(2).compare("1") != 0 && command.at(2).compare("0") != 0 ) ){
            return "UNK 7";
        }
        activeDesks[--deskIt]->set_occupancy_state( command.at(2).compare("1") == 0 ? true : false);

        return "ack";

    }
    else if(command.at(0).compare("r") == 0){
        restart();
        return "ack";
    }
    else if(command.at(0).compare("b") == 0){

    }
	return "UNK 8";
}

void DataManager::restart(){}
