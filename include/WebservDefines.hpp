#ifndef WEBSERV_DEFINES_HPP
 #define WEBSERV_DEFINES_HPP

#include <string>
#include <vector>

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

// https://developer.mozilla.org/en-US/docs/Web/HTTP/Status
typedef enum
{
    ST_CONTINUE = 100,
    ST_OK = 200,
    ST_CREATED = 201,
    ST_ACCEPTED = 202,
    ST_MULTIPLE_CHOICES = 300,
    ST_MOVED_PERMANENTLY = 301,
    ST_FOUND = 302,
    ST_SEE_OTHER = 303,
    ST_BAD_REQUEST = 400,
    ST_UNAUTHORIZED = 401,
    ST_FORBIDDEN = 403,
    ST_NOT_FOUND = 404,
    ST_METHOD_NOT_ALLOWED = 405,
    ST_REQUEST_TIMEOUT = 408,
    ST_LENGTH_REQUIRED = 411,
    ST_PAYLOAD_TOO_LARGE = 413,
    ST_INTERNAL_SERVER_ERROR = 500,
    ST_NOT_IMPLEMENTED = 501,
    ST_BAD_GATEWAY = 502,
    ST_SERVICE_UNAVAILABLE = 503,
} StatusCode;


// FIXME
struct Location
{
    std::string location;
    std::string index;              // ex. index.html
    std::string root;               // ex ./myDir/...
    std::vector<MethodType> allowMethods;       // ex. GET POST DELETE ...
    int  ClientMaxBodySize;  // (--> max size of client body request)   --> defaults to 8000 bytes
    std::vector<std::string> cgiInfo;			// ex. name: cgi_tester, arg: hello_world
};

void printLog(const std::string& log, const std::string& color);
std::string encodePercentEncoding(const std::string& str);
std::string decodePercentEncoding(const std::string& encodedURI);

#endif
