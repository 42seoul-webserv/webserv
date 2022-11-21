#ifndef REQUESTPROCESSOR_HPP
#define REQUESTPROCESSOR_HPP

#include "WebservDefines.hpp"
#include "HttpResponse.hpp"
#include <map>

/*
    HttpResponse response;
    .. after this code, response data is set.
    ResponseProcessor(response).process() 하면 끝.

*/



class ResponseProcessor
{
private: // Member data to process
    HTTPResponse& _res;

private: // Callback for kevent
    void bodyFdReadHander(); // if fd is not -1, then read data
    void socektSendHandler(); // send string to socket

public:
    void process(const HTTPResponse& res);

public:
    ResponseProcessor(HTTPResponse& res)
        : _res(res)
    {

    }
    ~ResponseProcessor();
};

#endif //REQUESTPROCESSOR_HPP
