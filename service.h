#ifndef SERVICE_H
#define SERVICE_H

#include <string>
#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;

class Service
{
public:
    Service(int _port);
	Service(string _address, int _port);

    static size_t ReadComplete(char * buff, const error_code & err, size_t bytes);

    bool SendConnection();
	bool SendSymbol(char msg);
	bool SendMessage(string msg);

    bool HandleConnection();
    char HandleSymbol();
    string HandleMessage();

private:
    io_service io;

    string enemyAddress;
    int port;

    ip::tcp::acceptor acceptor;
    ip::tcp::endpoint endpoint;

	boost::system::error_code ec;
};

#endif // SERVICE_H
