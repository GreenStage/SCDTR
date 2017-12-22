
#include <iostream>
#include "server.hpp"
#include "defs.h"
#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>


boost::asio::io_service * service = NULL;
void end( int signum ) {
   cout << "\nFinalizing the server" << endl;
   DataManager::getInstance()->exit = true;
   DataManager::terminate();
   if(service) service->stop();
}

int main(){

    try
    {
		service = new boost::asio::io_service();
		signal(SIGTERM, end);  
		signal(SIGKILL, end);  
		signal(SIGQUIT, end);  
		signal(SIGINT, end);  
		signal(SIGTSTP , end); 
        DataManager::initialize();

        Server * sv = new Server(*service,DataManager::getInstance());
        
		boost::asio::signal_set signals(*service, SIGINT, SIGTERM);
		signals.async_wait(boost::bind(&boost::asio::io_service::stop, service));
		
		service->run();
		end(0);
    }
    catch(std::exception& e){
       // std::cerr << e.what() << std::endl;
    }
    return 0;
}
