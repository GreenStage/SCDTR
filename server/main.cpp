
#include <iostream>
#include <boost/asio.hpp>

int main(){
    try
    {
        boost::asio::io_service io_service;
    }
    catch(std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    return 0;
}