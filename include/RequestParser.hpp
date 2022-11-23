#include "WebservDefines.hpp"
#include "HTTPRequest.hpp"
#include "ServerManager.hpp"
#include <map>
#include <string>

class RequestParser
{
private:
    void CRLFCheck(HTTPrequest* request);
    void getStartLine(HTTPrequest* request, size_t& end);
    void getHeader(HTTPrequest* request, size_t begin ,size_t endPOS);
    void bodyLengthCheck(HTTPrequest* request);
    void chunkedParsing(HTTPrequest* request);
    void bodyParsing(HTTPrequest* request);
    void startLineVaildCheck(HTTPrequest* request);
    void hearderVaildCheck(HTTPrequest* request);
    std::string::iterator getOneLine(std::string& str, \
                        std::string::iterator it, std::string::iterator end);
public:
  //  RequestParser();
   // ~RequestParser();
    void RequestParsing(FileDescriptor socektFD, HTTPrequest* request);
    void displayall(HTTPrequest* request);
};

