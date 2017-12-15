
#include "server.hpp"
#include "utils.hpp"

Server::Server(boost::asio::io_service& io_service,DataManager * dManager)
    : acceptor_( io_service, tcp::endpoint(tcp::v4(), 13) ){
    it = activeSessions.begin();
    dManager_ = dManager;
    start_accept();
}

void Server::start_accept(){
    Session::pointer new_connection =
        Session::create(acceptor_.get_io_service());

    acceptor_.async_accept(new_connection->get_socket(),
        boost::bind(&Server::handle_accept, this, new_connection,boost::asio::placeholders::error));
}

void Server::handle_accept(Session::pointer new_connection, const boost::system::error_code& error) {
    if (!error) {
        new_connection->start();
        activeSessions.insert(it,new_connection);

        start_accept();
    }
}


Session::pointer Session::create(boost::asio::io_service& io_service) {
    return Session::pointer(new Session(io_service));
}

void Session::start(){
        message_= {'o','i'};
        boost::asio::async_write(socket_, boost::asio::buffer(message_),
          boost::bind(&Session::handle_write, shared_from_this(),
             boost::asio::placeholders::error,
             boost::asio::placeholders::bytes_transferred)
        );
        boost::asio::async_read(socket_,boost::asio::buffer(message_read_,100),
            boost::bind(&Session::handle_read,shared_from_this(),boost::asio::placeholders::error)
        );

    }

Session::Session(boost::asio::io_service& io_service) : socket_(io_service){

}
    
void Session::handle_write(const boost::system::error_code& error,
    size_t bytes_transferred){
    /*cout << "WRITE: " << &&message_ << endl;*/
}

void Session::write(string message){
      boost::system::error_code ignored_error;
      boost::asio::write(socket, boost::asio::buffer(message),
          boost::asio::transfer_all(), ignored_error);
}
        
void Session::handle_read(const boost::system::error_code& error){
    if(error){
		cout << "Error occurred." << std::endl;

        return;
    }
    string message_str(message_read_.begin(),message_read_.end());
            std::cout << "Message: " << message_str << std::endl;
	string message_read(buf.data());
    std::cout << "READ: " << message_read << endl;
    dManager->parse_command(explode(message_read,' '));

    boost::asio::async_read(socket_,boost::asio::buffer(message_read_,100),
        boost::bind(&Session::handle_read,shared_from_this(),boost::asio::placeholders::error)
    );
}

tcp::socket&  Session::get_socket(){
	return socket_;
}

