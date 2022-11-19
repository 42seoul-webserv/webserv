#ifndef REQUESTPROCESSOR_HPP
#define REQUESTPROCESSOR_HPP

#include "ServerManager.hpp"
#include "WebservDefines.hpp"
#include <map>

class HttpResponse;
class HttpRequest;

/// @brief Using singleton object (?)

class RequestProcessor
{
private:

private:
    // ServerManager& _serverManager;
    void processGETMethod(const HttpRequest& req, struct Context* context);
    void processPOSTMethod(const HttpRequest& req, struct Context* context);
    void processPUTMethod(const HttpRequest& req, struct Context* context);
    void processDELETEMethod(const HttpRequest& req, struct Context* context);
    void processHEADMethod(const HttpRequest& req, struct Context* context);
    void processPATCHMethod(const HttpRequest& req, struct Context* context);
    void processCGI(const HttpRequest& req, struct Context* context);

public:
    void processRequest(const HttpRequest& req, struct Context* context);
    RequestProcessor();
    ~RequestProcessor();
};

#endif //REQUESTPROCESSOR_HPP
