
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
    static void initialize(int numDesks);
    static DataManager * getInstance();

    string parse_command(vector<string> command);
    int address;
    ~DataManager();

private:

    typedef struct __attribute__ ((packed)) req_ard{
        uint8_t address;
        uint8_t type;
    } request_to_arduino;


    typedef struct __attribute__ ((packed)) _arduino_default_resp{
        uint8_t address;
        uint8_t type;
    } arduino_default_resp;

    list<request_to_arduino> pending_requests;
    list<request_to_arduino>::iterator rqsts_it;

    list<request_to_arduino> pending_responses;
    list<request_to_arduino>::iterator resps_it;

    void request(int , int );
    void parseResponse();
    string fetch_response(int ,int ,int );

    bool exit;

    DataManager(int numDesks);
    int serialFd;
    void restart();
    list<float> get_last_minute_buffer();
    void start_stream();
    void stop_stream();
    void parseRequest();
    static DataManager * instance;
    static bool initiliazed;
    int numDesks_;
    Desk ** activeDesks;
    unsigned char buffer[60] = {0};

    Server * comm_;
};

#endif
