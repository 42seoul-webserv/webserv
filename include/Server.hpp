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
    std::pair<StatusCode, std::string> _redirect;
    Location* getMatchedLocation(const HTTPRequest& req);
    void processRequest(struct Context* context);
    void openServer();

    // check if server has valid redirection setting. (1. 서버 자체가 리다이렉션인지도 체크)
    bool isRedirect(const std::string& url, std::pair<StatusCode, std::string>* redir_buf) const;
    std::string getRealFilePath(const HTTPRequest& req);
public:
    Server();
    ~Server();
private:
    HTTPResponse* processGETRequest(const struct Context* context);
    HTTPResponse* processPOSTRequest(struct Context* context);
    HTTPResponse* processPUTRequest(struct Context* context);
    HTTPResponse* processDELETERequest(const struct Context* context);
    HTTPResponse* processHEADRequest(const struct Context* context);
};

#endif
