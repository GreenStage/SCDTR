


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

    ~DataManager();

private:
    DataManager(int numDesks);

    void restart();
    list<float> get_last_minute_buffer();
    void start_stream();
    void stop_stream();
    void validate_data();
    static DataManager * instance;
    static bool initiliazed;
    int numDesks_;
    Desk * activeDesks;

    thread timerThread;
};

#endif