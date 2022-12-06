#ifndef CGI_HPP
# define CGI_HPP
#include <unistd.h>
#include <cstdlib>
#include "ServerManager.hpp"
#include "HTTPResponse.hpp"
# define ENVCOUNT 60
//void pipeWriteHandler(); -> serverutil
//void CGIChildHandler();
class CGI
{
  public:
    pid_t pid;
    size_t envCount;
    FileDescriptor writeFD;
    FileDescriptor readFD;
    std::string writeFilePath;
    std::string readFilePath;
    int exitStatus;
    char** env;
    char** cmd;
    char* path;



    std::string getQueryFullPath(HTTPRequest& req);
    static std::string ft_getcwd();
    void parseStartLine(struct Context* context, std::string &message);
    void parseHeader(HTTPResponse* res, std::string &message);
    void parseBody(HTTPResponse* res, size_t count);
    void parseCGI(struct Context* context);
    void closeProcess(); //child수거?, response 생성?
    void setFilePath(CGI* cgi); // fork, pipe init
    void setCGIenv(Server server, HTTPRequest& req, struct Context* context);
    void getPATH(Server server, HTTPRequest& req);
    void setRequestEnv(HTTPRequest& req);
    void addEnv(std::string key, std::string val);
    void attachFileWriteEvent(struct Context* context);
    void CGIChildEvent(struct Context* context);
    void CGIfork(struct Context* context);
    CGI();
    ~CGI();
};
void CGIProcess(struct Context* context); //processinit, kevent(fd), 
bool isCGIRequest(const std::string& file, Location* loc);//cgi 확인
#endif