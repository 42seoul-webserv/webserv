#include "WebservDefines.hpp"
#include "HTTPRequest.hpp"
#include "ServerManager.hpp"
#include <map>
#include <string>

class RequestParser
{
private:
    void checkCRLF(HTTPRequest* request);
    void getStartLine(HTTPRequest* request, size_t& end);
    void getQuery(HTTPRequest* request);
    void getHeader(HTTPRequest* request, size_t begin, size_t endPOS);
    void checkBodyLength(HTTPRequest* request);
    void parseChunked(HTTPRequest* request);
    void parseBody(HTTPRequest* request);
    void checkStartLineValid(HTTPRequest* request);
    void checkHeaderValid(HTTPRequest* request);
    void readRequest(FileDescriptor fd, HTTPRequest* request);
    std::string::iterator getOneLine(std::string& str, \
                        std::string::iterator it, std::string::iterator end);
public:
    //  RequestParser();
    // ~RequestParser();
    void parseRequest(struct Context* context);
    void displayAll(HTTPRequest* request);
};

