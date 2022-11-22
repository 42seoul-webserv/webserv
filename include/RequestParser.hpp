#include "WebservDefines.hpp"
#include "HTTPRequest.hpp"
#include "ServerManager.hpp"
#include <map>
#include <string>

class RequestParser
{
private:

public:
  //  RequestParser();
   // ~RequestParser();
    void RequestParsing(FileDescriptor socektFD, HTTPrequest* request);
    RequestStatus getStatusType(std::string& status);
    void CRLFCheck(HTTPrequest* request);
    void getStartLine(HTTPrequest* request, size_t& end);
    void eraseSpace(std::string& message, size_t begin);
    void getHeader(HTTPrequest* request, size_t begin ,size_t endPOS);
    void getNomalBody(HTTPrequest* request);
    void getChunkedBody(HTTPrequest* request);
    void bodyParsing(HTTPrequest* request);
    void startLineVaildCheck(HTTPrequest* request);
    void hearderVaildCheck(HTTPrequest* request);
//    void getOneLine(std::string& buffer, std::string& message, size_t POS);
    void versionVaildCheck(HTTPrequest* request);
    std::string::iterator getOneLine(std::string& str, \
                        std::string::iterator it, std::string::iterator end);
    void displayall(HTTPrequest* request);
};

