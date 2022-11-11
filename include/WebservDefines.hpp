#ifndef WEBSERV_DEFINES_HPP
 #define WEBSERV_DEFINES_HPP

#include <string>

typedef enum
{
    UNDEFINED = -1,
    GET = 0,
    POST = 1,
    PUT = 2,
    PATCH = 3,
    DELETE = 4
} MethodType;

MethodType getMethodType(const std::string& method);
typedef unsigned int Port;
typedef int FileDescriptor;

#endif
