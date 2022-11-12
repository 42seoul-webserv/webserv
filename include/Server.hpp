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

// constructor, destructor
public:
    Server();
    ~Server();
    void openServer();
};

#endif
