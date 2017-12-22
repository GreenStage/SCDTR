
#ifndef DATA_HEADER
#define DATA_HEADER

#include <thread>
#include <list>
#include <vector>
#include "defs.h"
#include "desk.hpp"


using namespace std;


class DataManager{

public:
    static void initialize();
    static DataManager * getInstance();
    static void terminate();
    string parse_command(function<void(string)> wt,vector<string> command);
    int address;

    int exit = 0;
private:
	void registerStreamReceveiver(function<void(string)> wt,int packetId, int desk);
	void unRegisterStreamReceveiver(int packet_id,int desk);
	void i2c_write(packet_t rqst);
	bool streaming[5][256];
	double timeStreamStarted[5][256];
	
	function<void(string)>  writer = NULL;

    list<packet_t> pending_requests;
    list<packet_t>::iterator rqsts_it;

    list<packet_t*> pending_responses;
    list<packet_t*>::iterator resps_it;
    packet_t ** request_all( int );
    packet_t * request(int , int );
    void i2c_listener();
    packet_t * fetch_response(int ,int ,int );

	
    ~DataManager();
    DataManager();
    int serialFd;
    void restart();
    list<float> get_last_minute_buffer();
    void start_stream();
    void stop_stream();
    void parseRequest();
    static DataManager * instance;


    static bool initiliazed;
    int numDesks_;
    int file_i2c=-1;
    int listeningTo;
    Desk ** activeDesks;
    unsigned char buffer[60] = {0};
	void desks_detect();
	void desks_send_network();
	int desk_of_addr(int addr);
};

#endif
