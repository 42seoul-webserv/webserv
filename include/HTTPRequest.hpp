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
    std::string _message;
    std::string _body;
    MethodType _method;
    std::string _qury;
    std::string _url;
    std::string _version;
    std::map<std::string, std::string> _headers;
    bool _chunckedFlag;
    RequestStatus _status;
    CheckLevel _checkLevel;
    HTTPRequest()
    {
      _chunckedFlag = false;
      _method = UNDEFINED;
      _status = READING;
      _checkLevel = CRLF;
    }
} HTTPRequest;

#endif 