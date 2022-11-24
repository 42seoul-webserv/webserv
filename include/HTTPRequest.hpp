#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include <string>
#include <map>
#include "WebservDefines.hpp"

typedef enum
{
    END,
    HEADEROK,
    READING,
    ERROR
} RequestStatus;

typedef enum
{
    CRLF,
    STARTLINE,
    HEADER,
    BODY
} CheckLevel;

typedef struct HTTPRequest
{
    std::string message; // request Message ( all )
    std::string body;
    MethodType method;
    std::map<std::string, std::string> query; // url?query
    std::string url; // pure url
    std::string version;
    std::map<std::string, std::string> headers;
    bool chunkedFlag;
    RequestStatus status;
    CheckLevel checkLevel;

    HTTPRequest()
    {
      chunkedFlag = false;
      method = UNDEFINED;
      status = READING;
      checkLevel = CRLF;
    }
} HTTPRequest;

#endif 