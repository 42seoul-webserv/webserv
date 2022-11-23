#include "WebservDefines.hpp"
#include "HTTPRequest.hpp"
#include "ServerManager.hpp"
#include <map>
#include <string>

class RequestParser
{
private:
    void CRLFCheck(HTTPRequest* request);
    void getStartLine(HTTPRequest* request, size_t& end);
    void getHeader(HTTPRequest* request, size_t begin ,size_t endPOS);
    void bodyLengthCheck(HTTPRequest* request);
    void chunkedParsing(HTTPRequest* request);
    void bodyParsing(HTTPRequest* request);
    void startLineVaildCheck(HTTPRequest* request);
    void hearderVaildCheck(HTTPRequest* request);
    std::string::iterator getOneLine(std::string& str, \
                        std::string::iterator it, std::string::iterator end);
public:
  //  RequestParser();
   // ~RequestParser();
    void RequestParsing(FileDescriptor socektFD, HTTPRequest* request);
    void displayall(HTTPRequest* request);
};

