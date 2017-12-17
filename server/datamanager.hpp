
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
    static void initialize(int numDesks,int *desk_address);
    static DataManager * getInstance();

    string parse_command(vector<string> command);
    int address;
    ~DataManager();

private:

    typedef struct req_ard{
		uint8_t src_address;
        uint8_t dest_address;
        uint8_t type;
    } __attribute__((__packed__)) request_to_arduino;


    typedef struct _arduino_default_resp{
		uint8_t src_address;
        uint8_t address;
        uint8_t type;
    } __attribute__((__packed__)) arduino_default_resp;

    list<request_to_arduino> pending_requests;
    list<request_to_arduino>::iterator rqsts_it;

    list<arduino_default_resp> pending_responses;
    list<arduino_default_resp>::iterator resps_it;

    string request(int , int );
    void parseResponse();
    string fetch_response(int ,int ,int );

    bool exit = false;

    DataManager(int numDesks,int * desk_address);
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

};

#endif
