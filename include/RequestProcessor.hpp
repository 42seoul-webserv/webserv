#ifndef REQUESTPROCESSOR_HPP
#define REQUESTPROCESSOR_HPP

#include "ServerManager.hpp"
#include "WebservDefines.hpp"
#include <map>

class HTTPResponse;
class HTTPRequest;

class RequestProcessor
{
private:
    ServerManager& _serverManager;
    RequestProcessor();
    ~RequestProcessor();
    HTTPResponse processGETMethod(const HTTPRequest& req, struct Context* context);
    HTTPResponse processPOSTMethod(const HTTPRequest& req, struct Context* context);
    HTTPResponse processPUTMethod(const HTTPRequest& req, struct Context* context);
    HTTPResponse processDELETEMethod(const HTTPRequest& req, struct Context* context);
    HTTPResponse processHEADMethod(const HTTPRequest& req, struct Context* context);
    HTTPResponse processPATCHMethod(const HTTPRequest& req, struct Context* context);
    HTTPResponse processCGI(const HTTPRequest& req, struct Context* context);
public:
    HTTPResponse processRequest(const HTTPRequest& req, struct Context* context);
};

#endif //REQUESTPROCESSOR_HPP
