#ifndef SERVER_HPP
#define SERVER_HPP

#include "WebservDefines.hpp"
#include "HttpResponse.hpp"
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
    HttpResponse& processGETRequest(const struct Context* context);
    HttpResponse& processPOSTRequest(const struct Context* context);
    HttpResponse& processPUTRequest(const struct Context* context);
    HttpResponse& processDELETERequest(const struct Context* context);
    HttpResponse& processPATCHRequest(const struct Context* context);
    HttpResponse& processHEADRequest(const struct Context* context);
    FileDescriptor getRequestFile(const HTTPRequest& req); // 단순히 해당 url을 체크해서 파일이 존재하는지 확인...?
};

#endif
