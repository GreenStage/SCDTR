
#include <iostream>
#include <boost/asio.hpp>
#include "server.hpp"

int main(){
    try
    {
        boost::asio::io_service io_service;
        Server * sv = new Server();
    }
    catch(std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    return 0;
}