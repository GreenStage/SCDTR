
#include <iostream>
#include "server.hpp"

int main(){
    try
    {
        boost::asio::io_service io_service;
        Server * sv = new Server(io_service,DataManager::getInstance());
    }
    catch(std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
