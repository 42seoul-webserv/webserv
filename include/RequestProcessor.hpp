#ifndef REQUESTPROCESSOR_HPP
#define REQUESTPROCESSOR_HPP

#include "WebservDefines.hpp"
#include "HttpResponse.hpp"
#include <map>

class ServerManager;
class HTTPRequest;
// Request Processor
// HTTPRequest 를 바탕으로 해당 Request를 처리함
// 서버의 메인 로직은 해당 Request Proccessor 에서 모두 처리함.
class RequestProcessor
{
private:
    StatusCode isValidHeader(const HTTPRequest& req);
    HttpResponse* createResponse(const HTTPRequest& req);
public:
    void processRequest(struct Context* context);
// attributes
private:
    ServerManager& _serverManager;
};

#endif //REQUESTPROCESSOR_HPP
