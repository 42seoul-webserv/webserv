#ifndef SERVER_HPP
#define SERVER_HPP

#include "WebservDefines.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <sys/event.h>
#include <fcntl.h>
#include <vector>
#include <string>

class HTTPRequest;
class HTTPResponse;

class Server
{
// attributes
public:
    FileDescriptor _serverFD;
    struct sockaddr_in _socketAddr;
    std::string _index;
    std::string _root;
    std::vector<MethodType> _allowMethods;
    std::vector<Location> _locations;
    HTTPResponse& processGETRequest(const HTTPRequest& req);
    HTTPResponse& processPOSTRequest(const HTTPRequest& req);
    HTTPResponse& processPUTRequest(const HTTPRequest& req);
    HTTPResponse& processDELETERequest(const HTTPRequest& req);
    HTTPResponse& processPATCHRequest(const HTTPRequest& req);
    HTTPResponse& processHEADRequest(const HTTPRequest& req);
public:
    Server();
    ~Server();
    void openServer();
    HTTPResponse& processRequest(const HTTPRequest& req);
};

#endif
