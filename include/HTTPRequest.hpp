//
// Created by Sungjun Park on 2022/11/21.
//

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <map>
#include <string>

typedef struct
{
    std::string url;
    MethodType method;
    std::string version;
    std::map<std::string, std::string> header;
    std::string body;
} HTTPRequest;

#endif //HTTPREQUEST_HPP
