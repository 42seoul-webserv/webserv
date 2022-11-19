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
    void processGETMethod(const HTTPRequest& req, struct Context* context);
    void processPOSTMethod(const HTTPRequest& req, struct Context* context);
    void processPUTMethod(const HTTPRequest& req, struct Context* context);
    void processDELETEMethod(const HTTPRequest& req, struct Context* context);
    void processHEADMethod(const HTTPRequest& req, struct Context* context);
    void processPATCHMethod(const HTTPRequest& req, struct Context* context);
    void processCGI(const HTTPRequest& req, struct Context* context);
public:
    void processRequest(const HTTPRequest& req, struct Context* context);
};

#endif //REQUESTPROCESSOR_HPP
