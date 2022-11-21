#ifndef REQUESTPROCESSOR_HPP
#define REQUESTPROCESSOR_HPP

#include "ServerManager.hpp"
#include "WebservDefines.hpp"
#include <map>

class HTTPResponse;
class HTTPRequest;

// Request Processor
// HTTPRequest 를 바탕으로 해당 Request를 처리함
// 서버의 메인 로직은 해당 Request Proccessor 에서 모두 처리함.
class RequestProcessor
{
private:
    StatusCode isValidHeader(const HTTPRequest& req);
    HTTPResponse* createResponse(const HTTPRequest& req);
public:
    void processRequest(struct Context* context);
// attributes
private:
    ServerManager& _serverManager;
};

#endif //REQUESTPROCESSOR_HPP
