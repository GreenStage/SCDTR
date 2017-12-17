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
    void write(string message);
    void start();
	tcp::socket& get_socket();
	typedef boost::shared_ptr<Session> pointer;
	static pointer create(boost::asio::io_service& io_service,DataManager * dManager);
private:

    Session(boost::asio::io_service& service,DataManager * dManager);


	void handle_read(const boost::system::error_code& error,size_t bytes_read);

    void handle_write(const boost::system::error_code& /*error*/, size_t /*bytes_transferred*/);

    tcp::socket socket_;

    boost::asio::streambuf input_buffer_;
    DataManager * dManager_;
};

class Server{

public:
    Server(boost::asio::io_service& io_service,DataManager* dManager);

private:
    void start_accept();
    void handle_accept(Session::pointer, const boost::system::error_code&);
	DataManager * dManager_;
    list<Session::pointer> activeSessions;
    list<Session::pointer>::iterator it;
    tcp::acceptor acceptor_;
};


#endif

