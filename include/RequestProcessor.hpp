#ifndef REQUESTPROCESSOR_HPP
#define REQUESTPROCESSOR_HPP

#include "WebservDefines.hpp"
#include "HTTPRequest.hpp"
#include <map>

class ServerManager;

// Request Processor
// HTTPRequest 를 바탕으로 해당 Request를 처리함
// 서버의 메인 로직은 해당 Request Proccessor 에서 모두 처리함.
class RequestProcessor
{
private:
    StatusCode checkValidHeader(const HTTPRequest &req);

public:
    void processRequest(struct Context *context);
    explicit RequestProcessor(ServerManager &svm);

private:
    ServerManager &_serverManager;
};

#endif //REQUESTPROCESSOR_HPP
