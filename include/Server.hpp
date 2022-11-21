#ifndef SERVER_HPP
#define SERVER_HPP

#include "WebservDefines.hpp"
#include "HttpResponse.hpp"
#include <map>
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

class Server
{
// attributes
public:
    FileDescriptor _serverFD;
    struct sockaddr_in _socketAddr;
    std::string _index;
    std::string _root;
    std::map<StatusCode, std::string> _errorPage;
    std::vector<MethodType> _allowMethods;
    std::vector<Location> _locations;
    std::string _serverName;
    int _serverPort;
    HttpResponse& processGETRequest(const HTTPRequest& req);
    HttpResponse& processPOSTRequest(const HTTPRequest& req);
    HttpResponse& processPUTRequest(const HTTPRequest& req);
    HttpResponse& processDELETERequest(const HTTPRequest& req);
    HttpResponse& processPATCHRequest(const HTTPRequest& req);
    HttpResponse& processHEADRequest(const HTTPRequest& req);
public:
    Server();
    ~Server();
    void openServer();
    HttpResponse& processRequest(const HTTPRequest& req);
};

#endif
