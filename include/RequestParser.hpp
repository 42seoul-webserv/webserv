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
    std::string::iterator getOneLine(std::string& str, \
                        std::string::iterator it, std::string::iterator end);
public:
    //  RequestParser();
    // ~RequestParser();
    void parseRequest(FileDescriptor socketFD, HTTPRequest* request);
    void displayAll(HTTPRequest* request);
};

