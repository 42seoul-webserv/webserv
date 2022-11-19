#ifndef SERVER_HPP
#define SERVER_HPP

#include "WebservDefines.hpp"
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

typedef int StatusCode;

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
    std::string _server_name;

// constructor, destructor
public:
    Server();
    ~Server();
    void openServer();
};

#endif
