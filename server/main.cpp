
#include <iostream>
#include "server.hpp"
#include "defs.h"

int main(){
    int desk_addresses[] = {
        0x1,
        0x2,
        0x3
    }
    try
    {
        boost::asio::io_service io_service;
        DataManager::initialize(ARRAYLENGTH(desk_addresses),desk_addresses);
        Server * sv = new Server(io_service,DataManager::getInstance());
    }
    catch(std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
