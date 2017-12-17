
#include <iostream>
#include "server.hpp"
#include "defs.h"

int main(){
    int desk_addresses[] = {
        0x0f,
        0x10
    };
    try
    {

        boost::asio::io_service io_service;
  
        DataManager::initialize(2,desk_addresses);

        Server * sv = new Server(io_service,DataManager::getInstance());
		io_service.run();
		
    }
    catch(std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
