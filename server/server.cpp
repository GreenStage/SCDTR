
#include "server.hpp"
#include "defs.h"
#include "utils.hpp"

#define SERVER_TCP_PORT 13 


Server::Server(boost::asio::io_service& io_service,DataManager * dManager)
    : acceptor_( io_service, tcp::endpoint(tcp::v4(), SERVER_TCP_PORT) ){
    it = activeSessions.begin();
    printf("Server listening on %d\n",SERVER_TCP_PORT);
    dManager_ = dManager;
    start_accept();
}

void Server::start_accept(){
	
    Session::pointer new_connection = Session::create(acceptor_.get_io_service(),dManager_);

    acceptor_.async_accept(new_connection->get_socket(),
        boost::bind(&Server::handle_accept, this, new_connection,boost::asio::placeholders::error));
}

void Server::handle_accept(Session::pointer new_connection, const boost::system::error_code& error) {
#ifdef DEBUG
	printf("Handle accept \n");
#endif
	if (!error) {

        new_connection->start();

        activeSessions.insert(it,new_connection);

        start_accept();
    }
    else printf("%s\n",error.message().c_str());
}


Session::pointer Session::create(boost::asio::io_service& io_service,DataManager * dManager) {
    return Session::pointer(new Session(io_service,dManager));
}

void Session::start(){
		write("Hello");

		boost::asio::async_read_until(socket_,input_buffer_,'\n',
            boost::bind(&Session::handle_read,shared_from_this(),boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred)
        );
}

Session::Session(boost::asio::io_service& io_service,DataManager * dManager) : socket_(io_service){
	dManager_ = dManager;
}
    
void Session::handle_write(const boost::system::error_code& error,
    size_t bytes_transferred){
	if(!error){
		cout << "Sent: " << bytes_transferred << endl;
	}
	else cout << "Error sending message: " << error.message() << endl;

}

void Session::write(string message){

    boost::asio::async_write(socket_, boost::asio::buffer(message.append("\n")),
        boost::bind(&Session::handle_write, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
}
        
void Session::handle_read(const boost::system::error_code& error,size_t bytes_read){
	string message_read_;
	function <void(string)> fp = std::bind(&Session::write,this,placeholders::_1);
	
    if(error &&  error != boost::asio::error::eof){
		cout << "Error occurred: " << error.message() << std::endl;
        return;
    }
	istream is(&input_buffer_);
    getline(is, message_read_);


    if (!message_read_.empty()){
#ifdef DEBUG

		cout << "Received: " << message_read_ << endl;
#endif
		try{
			message_read_ = dManager_->parse_command(fp,explode(message_read_,' '));
			cout << "ts" << message_read_ << endl;
			write(message_read_);
		}catch(string e ){
			cout << "Error" << e << endl;
		}
		
	}
	else write("");

	boost::asio::async_read_until(socket_,input_buffer_,'\n',
		boost::bind(&Session::handle_read,shared_from_this(),boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred)
	);
}

tcp::socket&  Session::get_socket(){
	return socket_;
}

