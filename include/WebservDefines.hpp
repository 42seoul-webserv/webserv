#ifndef WEBSERV_DEFINES_HPP
 #define WEBSERV_DEFINES_HPP

#include <string>

#define BUFFER_SIZE 8192
#define LISTEN_QUEUE_SIZE 1024

// colors
#define PRINT_RED     "\x1b[31m"
#define PRINT_GREEN   "\x1b[32m"
#define PRINT_YELLOW  "\x1b[33m"
#define PRINT_BLUE    "\x1b[34m"
#define PRINT_MAGENTA "\x1b[35m"
#define PRINT_CYAN    "\x1b[36m"
#define PRINT_RESET   "\x1b[0m"

typedef enum
{
    GET = 0,
    POST = 1,
    PUT = 2,
    PATCH = 3,
    DELETE = 4,
    UNDEFINED = 9
} MethodType;

MethodType getMethodType(const std::string& method);
typedef unsigned int Port;
typedef int FileDescriptor;

// FIXME
struct Location
{
    // std::string _location;
    // std::string _index;              // ex. index.html
    // std::string _root;               // ex ./myDir/...
    // std::vector<MethodType> _allowMethods;       // ex. GET POST DELETE ...
    // int  _clientRequestBodyMaxSize;  // (--> max size of client body request)   --> defaults to 8000 bytes
    // //t_cgiInfo		_cgiInfo;			// ex. name: cgi_tester, arg: hello_world
};

void printLog(const std::string& log, const std::string& color);


#endif
