
#include "server.hpp"
#include "datamanager.hpp"
#include "utils.hpp"

server::server(boost::asio::io_service& io_service)
    : acceptor_( io_service, tcp::endpoint(tcp::v4(), 13) ){
    
    start_accept(dManager);
}

void server::start_accept(){
    Session::pointer new_connection =
        Session::create(acceptor_.io_service());

    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&tcp_server::handle_accept, this, new_connection,
            boost::asio::placeholders::error));
}

void server::handle_accept(Session::pointer new_connection, const boost::system::error_code& error) {
    if (!error) {
        new_connection->start();
        activeSessions.insert(new_connection);

        start_accept();
    }
}


pointer Session::create(boost::asio::io_service& io_service) {
    return pointer(new Session(io_service));
}

void Session::start(){
        message_ = "oi";
        boost::asio::async_write(socket_, boost::asio::buffer(message_),
          boost::bind(&Session::handle_write, shared_from_this(),
             boost::asio::placeholders::error,
             boost::asio::placeholders::bytes_transferred)
        );
        boost::asio::async_read(socket_,boost:assio::buffer(message_read_),
            boost:bind(&Session:handle_read,shared_from_this(),
                boost:assio:placeholders::error,
                boost::asio::placeholders::bytes_tranferred)
        );

    }

Session::Session(boost::asio::io_service& io_service) : socket_(io_service){

}
    
void Session:handle_write(const boost::system::error_code& error,
    size_t bytes_transferred){
    std::cout << "WRITE: " << message_ << endl;
}

void Session::handle_read(cconst boost::system::error_code& error,
    size_t bytes_transferred){

    std::cour << "READ: " << message_read_ << endl;
    dManager.parse_command(explode(message_read,' '));

    boost::asio::async_read(socket_,boost:assio::buffer(message_read_),
        boost:bind(&Session:handle_read,shared_from_this(),
            boost:assio:placeholders::error,
            boost::asio::placeholders::bytes_tranferred)
    );
}
