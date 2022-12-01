#ifndef CGI_HPP
# define CGI_HPP
#include <unistd.h>
#include <cstdlib>
#include "ServerManager.hpp"
#include "HTTPResponse.hpp"
# define ENVCOUNT 40
//void pipeWriteHandler(); -> serverutil
//void CGIChildHandler();
class CGI
{
  public:
    pid_t pid;
    size_t envCount;
    FileDescriptor writeFD;
    FileDescriptor readFD;
    int exitStatus;
    char** env;
    char** cmd;
    char* path;

    std::string getQueryFullPath(HTTPRequest& req);
    std::string getCWD();
    static void parseStartLine(struct Context* context, std::string &message);
    static void parseHeader(HTTPResponse* res, std::string &message);
    static void parseBody(HTTPResponse* res, std::string message);
    static void parseCGI(struct Context* context);
    void closeProcess(); //child수거?, response 생성?
    void processInit(CGI* cgi); // fork, pipe init
    void setCGIenv(Server server, HTTPRequest& req, struct Context* context);
    void getPATH(Server server, HTTPRequest& req);
    void addEnv(std::string key, std::string val);
    void CGIEvent(struct Context* context);
    CGI();
    ~CGI();
};
void CGIProcess(struct Context* context); //processinit, kevent(fd), 
bool isCGIRequest(const std::string& file, Location* loc);//cgi 확인
#endif