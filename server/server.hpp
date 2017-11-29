#ifndef SERVER_HEADER
#define SERVER_HEADER

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "datamanager.hpp"
#include <list>
using boost::asio::ip::tcp;
using namespace std;

class server{

public:
    server(boost::asio::io_service& io_service,DataManager& dManager);
private:
    void start_accept();
    void handle_accept(Session::pointer new_connection, const boost::system::error_code& error);

    list<Session> activeSessions;
    tcp::acceptor acceptor_;
};


class Session : public boost::enable_shared_from_this<Session> {

public:
    typedef boost::shared_ptr<Session> pointer;
    static pointer create(boost::asio::io_service& io_service);
    void start();

private:

    Session(boost::asio::io_service& io_service) : socket_(io_service);

    void handle_read(const boost::system::error_code& /*error*/, size_t /*bytes_transferred*/);

    void handle_write(const boost::system::error_code& /*error*/, size_t /*bytes_transferred*/);

    tcp::socket socket_;
    std::string message_;
    std::string message_read_;
    DataManager dManager;
};
#endif

