#include "WebservDefines.hpp"
#include <map>
#include <string>

class RequestParser
{
private:
    std::map<FileDescriptor, std::string> _MessegeList;
public:
    RequestParser();
    ~RequestParser();
    bool RequestParsing();
    void addFD();
    void eraseFD();
    bool messegeCheck();
};
