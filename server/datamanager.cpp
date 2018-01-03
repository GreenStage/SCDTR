#include "datamanager.hpp"
#include <stdexcept>
#include <string>
#include <chrono>
#include <ctime>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <wiringSerial.h>
#include <pigpio.h>

#define RPI_SLAVE_ADDR 0x48
#define BROADCAST_ADDR 0x0
#define XFERCONTROL (RPI_SLAVE_ADDR<<16) | (0x01<<9) | (0x01<<8) | (0x01<<2) | (0x00<<1) | 0x01

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
	//NEW PACKETS -- Last Minute Buffers
	RASP_RQST_M_ILU,
	RASP_RQST_M_DUTY_CICLE,
	//NEW PACKETS -- Stream requests
	RASP_RQST_START_ILU,
	RASP_RQST_START_DUTY_CICLE,
	RASP_RQST_STOP_ILU,
	RASP_RQST_STOP_DUTY_CICLE,
	//NEW PACKETS -- Restart requests
	RASP_RQST_RESTART,
	RASP_RQST_TIME_RUNNING,
	RASP_RQST_MAX,

	ARD_RESP_MIN = 0x80,
	ARD_RESP_NETWORK,
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
	//NEW PACKETS -- Last Minute Buffers
	ARD_RESP_M_ILU,
	ARD_RESP_M_DUTY_CICLE,
	//NEW PACKETS -- Restart requests
	ARD_RESP_TIME_RUNNING,

	//Afonso
	ARD_RESP_BC_ADDR,
	ARD_RESP_DC,
	ARD_SYNC,
	ARD_CONSENSUS,
	ARD_RESP_MAX
};

int response_of(int id){
  switch(id){
    case RASP_RQST_ILU:					return ARD_RESP_ILU;
    case RASP_RQST_DUTY_CICLE:			return ARD_RESP_DUTY_CICLE;
    case RASP_RQST_LOWER_ILUMINANCE: 	return ARD_RESP_LOWER_ILUMINANCE;
    case RASP_RQST_ACC_ENERGY:        	return ARD_RESP_ACC_ENERGY;
    case RASP_RQST_ACC_CONFORT_ERR:   	return ARD_RESP_ACC_CONFORT_ERR;
    case RASP_RQST_ACC_CONFORT_VAR:   	return ARD_RESP_ACC_CONFORT_VAR;
    case RASP_RQST_POW_CONSUP:        	return ARD_RESP_POW_CONSUP;
    case RASP_RQST_EXT_ILU:           	return ARD_RESP_EXT_ILU;
    case RASP_RQST_ILU_CTR:           	return ARD_RESP_ILU_CTR;
    case RASP_RQST_OCCUPANCY_ST:				return ARD_RESP_OCCUPANCY_ST;
    case RASP_RQST_M_ILU:			  				return ARD_RESP_M_ILU;
    case RASP_RQST_M_DUTY_CICLE:	  		return ARD_RESP_M_DUTY_CICLE;
    case RASP_RQST_TIME_RUNNING:				return ARD_RESP_TIME_RUNNING;
	default:						  	return PACKET_NONE;
  }
}

int size_of_packet(packet_t * p){
	switch(p->packet_id){
		case ARD_RESP_ILU:
		case ARD_RESP_DUTY_CICLE:
		case ARD_RESP_LOWER_ILUMINANCE:
		case ARD_RESP_ACC_ENERGY:
		case ARD_RESP_ACC_CONFORT_ERR:
		case ARD_RESP_ACC_CONFORT_VAR:
		case ARD_RESP_POW_CONSUP:
		case ARD_RESP_EXT_ILU:
		case ARD_RESP_ILU_CTR:
		case RASP_RQST_TIME_RUNNING:
		  return sizeof(single_float_packet);

		case ARD_RESP_OCCUPANCY_ST:
		  return sizeof(single_byte_packet);

		case ARD_RESP_M_DUTY_CICLE:
		case ARD_RESP_M_ILU:
		{
			multiple_float_packet * pt = (multiple_float_packet *) p;
			return sizeof(multiple_float_packet) -  ( (MAX_BYTE_ARR_LENGTH - pt->n_data) * sizeof(float) );
		}
		case ARD_RESP_BC_ADDR:
		case ARD_SYNC:
			return sizeof(packet_t);
		case ARD_RESP_DC:
			return sizeof(single_byte_packet);
		case ARD_CONSENSUS:
		{
			multiple_float_packet * pt = (multiple_float_packet *) p;
			return sizeof(multiple_float_packet) -  ( (MAX_BYTE_ARR_LENGTH - pt->n_data) * sizeof(float) );
		}
		default:
		  return sizeof(packet_t);
  }
}

void DataManager::desks_detect(){
	int ret,i,k;
    packet_t rqst;
    rqst.packet_id = PACKET_NONE;
    rqst.src_address = RPI_SLAVE_ADDR;


	for(i = 0x4,k = 0; i <0x50; i++){
		if(i == RPI_SLAVE_ADDR) continue;
		if(file_i2c = i2cOpen(1, i, 0)  < 0) continue;
		rqst.dest_address = i;
		if( (ret = i2cWriteDevice(file_i2c, (char*) &rqst,sizeof(packet_t))) == 0){
			printf("Device detected at %d\n",i);
			activeDesks[k] = new Desk(k,i);
			k++;
		}
		numDesks_ = k;
		i2cClose(file_i2c);
	}

}

void DataManager::desks_send_network(){
	int i,ret,send_bytes = 0;
	multiple_byte_packet p;

	p.packet_id = ARD_RESP_NETWORK;
	p.src_address = RPI_SLAVE_ADDR;
	p.n_data = numDesks_ + 1;
	p.data[0] = RPI_SLAVE_ADDR;

	for(i = 0; i < numDesks_;i++){
		p.data[i + 1] = activeDesks[i]->address;
	}

	send_bytes = sizeof(packet_t) + i + 1 + 1;

	for(i = 0; i < numDesks_;i++){

		if(file_i2c = i2cOpen(1, activeDesks[i]->address, 0)  < 0) continue;

		p.dest_address = activeDesks[i]->address;

		if( (ret = i2cWriteDevice(file_i2c, (char*) &p,send_bytes)) != 0){
			printf("error writing to i2c %d\n",ret);
		}
		i2cClose(file_i2c);
	}
}

DataManager * DataManager::instance = NULL;
bool DataManager::initiliazed = false;

class dm_not_Initialized: public exception {

    virtual const char* what() const throw() {
      return "DataManager was not initialized.";
    }
} dmNotInitialized;

void DataManager::initialize(){
    DataManager::instance = new DataManager();

    return;
}

DataManager * DataManager::getInstance(){
    if(!DataManager::initiliazed){
        throw dmNotInitialized;
    }

    return DataManager::instance;
}

DataManager::DataManager(){
    char *filename = (char*)"/dev/i2c-1";

    activeDesks = (Desk **) malloc(sizeof(Desk*) * 50);

	if (gpioInitialise() < 0) {
		cout << "Error initializing gpio" <<endl;
		exit = 1;
	}
  for(int a = 0; a < 5; a++) memset(streaming[a],0,256);
	writer = NULL;
  this->exit = false;
	rqsts_it = pending_requests.begin();
	resps_it = pending_responses.begin();

	desks_detect();
	desks_send_network();

  thread i2cthread = thread(&DataManager::i2c_listener,this);

  initiliazed = true;

	i2cthread.detach();
}

packet_t ** DataManager::request_all(int type){
	int i,ret;
	static packet_t ** responses = (packet_t **) malloc(sizeof(packet_t*) * numDesks_);
	packet_t rqst;

	rqst.packet_id = type;
	memset(responses,0,sizeof(packet_t*) * numDesks_);
	for(i = 0; i < numDesks_;i++){

		rqst.src_address = RPI_SLAVE_ADDR;
		rqst.dest_address = activeDesks[i]->address;
		printf("Send all, now to %d\n",rqst.dest_address);

		file_i2c = i2cOpen(1, rqst.dest_address, 0);

		if( (ret = i2cWriteDevice(file_i2c, (char *) &rqst, sizeof(packet_t)))){
			throw "error writing to i2c: " + to_string(ret);
		}


		i2cClose(file_i2c);
		pending_requests.push_back(rqst);
	}

	for(i = 0; i < numDesks_;i++){
		responses[i] = fetch_response(i,0, response_of(type) );
	}

	return responses;
}

void DataManager::i2c_write(packet_t rqst){
	int ret;
	file_i2c = i2cOpen(1, rqst.dest_address, 0);

	if( (ret = i2cWriteDevice(file_i2c, (char *) &rqst, sizeof(packet_t)))){
		cout << "error writing to i2c" << endl;
		throw "error writing to i2c: " + to_string(ret);
	}


	i2cClose(file_i2c);
}
packet_t * DataManager::request(int deskId, int type){
	packet_t * retval;
    packet_t rqst;

    int ret,i,addr = -1;
    int dataCtr = 0;

    for(i = 0; i < numDesks_;i++){
        if(activeDesks[i]->deskId == deskId){
            addr = activeDesks[i]->address;
            rqst.src_address = RPI_SLAVE_ADDR;
            rqst.dest_address = activeDesks[i]->address;
            rqst.packet_id = type;
            break;
        }
    }

    if(i == numDesks_){
		throw "Desk specified does not exist";
	}
	i2c_write(rqst);
/* Initialize Master*/
	printf("Sent %d %d %d\n",rqst.src_address ,rqst.dest_address ,rqst.packet_id);


    pending_requests.push_back(rqst);
	return fetch_response(deskId,addr, response_of(type) );

}
int DataManager::desk_of_addr(int addr){
	int i;
	for(i = 0; i< numDesks_;i++){
			if(activeDesks[i]->address == addr) break;
	}
	return i;
}
packet_t * DataManager::fetch_response(int deskid,int deskAddr,int type){
    int timeoutCtr = 0;
    packet_t * retval;
    while(!this->exit){
		if(pending_responses.empty()){
			if(timeoutCtr > 60){
				break;
			}
			timeoutCtr++;
			this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		for(resps_it = pending_responses.begin(); resps_it != pending_responses.end(); ++resps_it){
			if( ((*resps_it)->src_address == deskAddr || deskAddr == 0) && (*resps_it)->packet_id == type){
	#ifdef DEBUG
				printf("Processed response from dm %d, address: %d, type: %d\n",deskid,(*resps_it)->src_address,type);
	#endif
				retval = (*resps_it);
				pending_responses.erase(resps_it);
				return retval;
			}
		}
		timeoutCtr++;
		this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	return NULL;

}
void DataManager::i2c_listener(){
	int r;
	packet_t * retval, * default_in_buff;
    int readBytes = 0,status;

	gpioSetMode(18 ,PI_ALT3);
	gpioSetMode(19, PI_ALT3);

	bsc_xfer_t xfer;

    while(!this->exit){
        memset(&xfer,0,sizeof(bsc_xfer_t));
        xfer.control = XFERCONTROL;

		while(xfer.rxCnt == 0 && !this->exit) status = bscXfer(&xfer);

		readBytes = 0;
		while( sizeof(packet_t) < xfer.rxCnt - readBytes && !this->exit){

			default_in_buff = ( packet_t*) malloc(xfer.rxCnt -readBytes);
			memcpy(default_in_buff, xfer.rxBuf + readBytes,xfer.rxCnt -readBytes);


			cout << "I2C Message "<<xfer.rxCnt <<"received from " << to_string(default_in_buff->src_address) << " id: " << to_string(default_in_buff->packet_id )<< endl;


			/*Got a streaming message*/
			if(streaming[desk_of_addr(default_in_buff->src_address)][default_in_buff->packet_id]){
				ard_single_float_ * p = (ard_single_float_ *) default_in_buff;
				string writm = "c ";
				int desk,j;
				switch(default_in_buff->packet_id){
					case ARD_RESP_ILU:
						writm += "l ";
						break;
					case ARD_RESP_DUTY_CICLE:
						writm += "d ";
						break;
				}

				j = desk_of_addr(default_in_buff->src_address);
				writm += to_string(j+1);
				writm += " ";
				writm += to_string(p->val);
				writm += " ";
				writm += to_string(time(0) - timeStreamStarted[j][default_in_buff->packet_id]);
				readBytes += sizeof(ard_single_float_);
				writer(writm);
				free(default_in_buff);
				continue;
			}


			for(rqsts_it = pending_requests.begin(); rqsts_it != pending_requests.end();){
				if(rqsts_it->dest_address == default_in_buff->src_address && default_in_buff->packet_id == response_of(rqsts_it->packet_id) ){
					rqsts_it = pending_requests.erase(rqsts_it);
					pending_responses.push_back(default_in_buff);
				}
				else rqsts_it++;
			}
			readBytes += size_of_packet(default_in_buff);


			/*Got more messages in buffer, read them*/
			if(readBytes < xfer.rxCnt){
				cout << "Remaining bytes in buffer: " << to_string(xfer.rxCnt -readBytes) << endl;
			}
		}
    }

	xfer.control = 0;
	status = bscXfer(&xfer);
    if (status >= 0) {
        printf("%d\n", status);
	}
	gpioTerminate();
	exit = 2;
	cout << "I2C listener thread finished." << endl;
}
void DataManager::terminate(){
	delete instance;
}
DataManager::~DataManager(){
	exit = 1;
	while(exit != 2);
    delete[] activeDesks;
	cout <<"Datamanager terminated." << endl;
}

void DataManager::registerStreamReceveiver(function<void(string)> wt,int packetId, int desk){
	streaming[desk][packetId] = true;
	timeStreamStarted[desk][packetId] = time(0);

	writer = wt;
}

void DataManager::unRegisterStreamReceveiver(int packet_id,int desk){
	streaming[desk][packet_id] = false;

}

string DataManager::parse_command(function<void(string)> wt,vector<string> command){
	packet_t * return_pck = NULL;
	int rqst_type;
	static string INVALID_ARG = "invalid command";

    string retval ="";

	if (command.at(0).compare("d") == 0){
		int deskIt,stopt;
		if(command.size() < 3){
            return "nak";
        }
		packet_t send_buf;

        if( ( deskIt = atoi(command.at(2).c_str() ) ) < 1 || deskIt > numDesks_) return "nak";
        deskIt--;

		if(command.at(1).compare("l") == 0){
			stopt = ARD_RESP_ILU;
			send_buf.packet_id = RASP_RQST_STOP_ILU;
		}

		else if(command.at(1).compare("d") == 0){
			stopt = ARD_RESP_DUTY_CICLE;
			send_buf.packet_id = RASP_RQST_STOP_DUTY_CICLE;
		}
		else return "nack";

		unRegisterStreamReceveiver(stopt,deskIt);

		send_buf.src_address =RPI_SLAVE_ADDR;
		send_buf.dest_address = activeDesks[deskIt]->address;

		i2c_write(send_buf);
		string ret =  "Stoping stream from desk "  + to_string(deskIt +1);
		return ret;
	}


    if(command.at(0).compare("g") == 0 || command.at(0).compare("c") == 0){
		if(command.size() < 3){
            return "nak";
        }
        retval = command.at(1);
        retval += " ";

        int deskIt;

        if(command.at(2).compare("T") == 0 ) {
            retval += "T ";
            switch(command.at(1).at(0)){
                case 'p': rqst_type = RASP_RQST_POW_CONSUP;			break;
                case 'e': rqst_type = RASP_RQST_ACC_ENERGY;			break;
                case 'c': rqst_type = RASP_RQST_ACC_CONFORT_ERR;	break;
                case 'v': rqst_type = RASP_RQST_ACC_CONFORT_VAR;	break;

				default:  return INVALID_ARG;
            }

            try{
				float data = 0;
				int i;
				single_float_packet * p;
				packet_t ** responses = request_all(rqst_type);
				for(i = 0; i < numDesks_; i++){

					p = (single_float_packet * )responses[i];
					if(!p) continue;

					data += p->val;
					free(p);
					responses[i] = NULL;
				}
				retval += to_string(data);
				cout << "T command response: " << retval << endl;
				return retval;
			}
			catch(string e){
				cout << e << endl;
				return e;
			}

        } else if( ( deskIt = atoi(command.at(2).c_str() ) ) > 0){
			int rqst_type;
			retval += to_string(deskIt);
            retval += " ";
            deskIt --;


            if(deskIt < 0 || deskIt > numDesks_){
                /*TODO*/
                return "UNK3";
            }

            switch(command.at(1).at(0)){

                case 'l': rqst_type = RASP_RQST_ILU;				break;
                case 'd': rqst_type = RASP_RQST_DUTY_CICLE;			break;
                case 'o': rqst_type = RASP_RQST_OCCUPANCY_ST;		break;
                case 'L': rqst_type = RASP_RQST_LOWER_ILUMINANCE; 	break;
                case 'O': rqst_type = RASP_RQST_EXT_ILU; 			break;
                case 'r': rqst_type = RASP_RQST_ILU_CTR; 			break;
                case 'p': rqst_type = RASP_RQST_POW_CONSUP;			break;
                case 'e': rqst_type = RASP_RQST_ACC_ENERGY;			break;
                case 'c': rqst_type = RASP_RQST_ACC_CONFORT_ERR;	break;
                case 'v': rqst_type = RASP_RQST_ACC_CONFORT_VAR;	break;

                default: return "invalid packet";
            }

            try{
				if(command.at(0).compare("c") == 0){
					packet_t send_buf;

					if(command.at(1).compare("l") == 0)
						send_buf.packet_id = RASP_RQST_START_ILU;
					else if(command.at(1).compare("d") == 0)
						send_buf.packet_id = RASP_RQST_START_DUTY_CICLE;
					else return "nack";

					registerStreamReceveiver(wt,response_of(rqst_type),deskIt);

					send_buf.dest_address = activeDesks[deskIt]->address;
					send_buf.src_address =RPI_SLAVE_ADDR;

					i2c_write(send_buf);
					string ret =  "Starting stream from desk "  + to_string(deskIt +1);
					return ret;
				}
				return_pck = request(deskIt,rqst_type);
				if(!return_pck)
					return "Timeout waiting for arduino's response";
			}
			catch(string e){
				cout << e << endl;
				return e;
			}

			switch(return_pck->packet_id){
				case ARD_RESP_ILU:
				case ARD_RESP_DUTY_CICLE:
				case ARD_RESP_LOWER_ILUMINANCE:
				case ARD_RESP_ACC_ENERGY:
				case ARD_RESP_ACC_CONFORT_ERR:
				case ARD_RESP_ACC_CONFORT_VAR:
				case ARD_RESP_POW_CONSUP:
				case ARD_RESP_EXT_ILU:
				case ARD_RESP_ILU_CTR:

					retval += to_string( (( single_float_packet*) return_pck)->val);

					break;
				case RASP_RQST_OCCUPANCY_ST:
					retval += to_string( (( single_byte_packet*) return_pck)->val);
					break;
			}


			free(return_pck);

        }
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
		int deskIt,j;
		if(command.size() < 3){
            return "nak";
        }
		if(command.at(1).compare("l") ==0){
			rqst_type =RASP_RQST_M_ILU;
		}
		else if(command.at(1).compare("d") == 0){
			rqst_type = RASP_RQST_M_DUTY_CICLE;
		}
		else return "nak";

		if( ( deskIt = atoi(command.at(2).c_str() ) ) < 1){
			return "nak";
		}
		deskIt--;
		return_pck = request(deskIt,rqst_type);
		retval = "b ";
		retval += command.at(1);
		retval +=" ";
		if(!return_pck)
			return "Timeout waiting for arduino's response";
		for(j = 0; j < (( multiple_float_packet*) return_pck)->n_data;j++){
			retval += to_string( (( multiple_float_packet*) return_pck)->data[j]);
			j++;
		}
		return retval;

    }

	cout << "ret " << retval << endl;
	return retval;
}

void DataManager::restart(){
	packet_t p;
	p.packet_id = RASP_RQST_RESTART;
}
