#ifndef SERVER_HPP
#define SERVER_HPP

#include "WebservDefines.hpp"
#include "HTTPRequest.hpp"
#include "Location.hpp"
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

class HTTPResponse;

class Server
{
// attributes (getter 사용 불필요하다고 생각해서 public처리)
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
    int _clientMaxBodySize;
    void processRequest(const struct Context* context);
    Location* getMatchedLocation(const HTTPRequest& req);
    void openServer();
public:
    Server();
    ~Server();
private:
    HTTPResponse& processGETRequest(const struct Context* context);
    HTTPResponse& processPOSTRequest(const struct Context* context);
    HTTPResponse& processPUTRequest(const struct Context* context);
    HTTPResponse& processDELETERequest(const struct Context* context);
    HTTPResponse& processPATCHRequest(const struct Context* context);
    HTTPResponse& processHEADRequest(const struct Context* context);
};

#endif
