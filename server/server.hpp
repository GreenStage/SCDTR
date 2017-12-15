#ifndef SERVER_HEADER
#define SERVER_HEADER
#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include <boost/array.hpp>

#include "datamanager.hpp"
#include <list>
using boost::asio::ip::tcp;
using namespace std;


class Session : public boost::enable_shared_from_this<Session> {

public:
   
    void start();
	tcp::socket& get_socket();
	typedef boost::shared_ptr<Session> pointer;
	static pointer create(boost::asio::io_service& io_service);
private:

    Session(boost::asio::io_service& service);


	void handle_read(const boost::system::error_code&);

    void handle_write(const boost::system::error_code& /*error*/, size_t /*bytes_transferred*/);

    tcp::socket socket_;
    boost::array<char, 100> message_;
    boost::array<char, 100> message_read_;
    DataManager * dManager;
};

class Server{

public:
    Server(boost::asio::io_service& io_service,DataManager* dManager);
    write(string message);
private:
    void start_accept();
    void handle_accept(Session::pointer, const boost::system::error_code&);
	DataManager * dManager_;
    list<Session::pointer> activeSessions;
    list<Session::pointer>::iterator it;
    tcp::acceptor acceptor_;
};


#endif

