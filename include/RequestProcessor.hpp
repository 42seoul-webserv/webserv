#ifndef REQUESTPROCESSOR_HPP
#define REQUESTPROCESSOR_HPP

#include "WebservDefines.hpp"
#include "HttpResponse.hpp"
#include <map>

class HttpRequest;

// Request Processor
// HTTPRequest 를 바탕으로 해당 Request를 처리함
// 서버의 메인 로직은 해당 Request Proccessor 에서 모두 처리함.
class RequestProcessor
{
private:
<<<<<<< HEAD
    StatusCode isValidHeader(const HTTPRequest& req);
    HTTPResponse* createResponse(const HTTPRequest& req);
public:
    void processRequest(struct Context* context);
// attributes
private:
    ServerManager& _serverManager;
=======
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
>>>>>>> develop
};

#endif //REQUESTPROCESSOR_HPP
