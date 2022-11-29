#ifndef CGI_HPP
# define CGI_HPP
#include <unistd.h>
#include <cstdlib>
#include "ServerManager.hpp"
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
    void closeProcess(); //child수거?, response 생성?
    void processInit(CGI* cgi); // fork, pipe init
    void setCGIenv(Server server, HTTPRequest& req, struct Context* context);
    void getPATH(Server server, HTTPRequest& req);
    void addEnv(std::string key, std::string val);
    void CGIEvent(struct Context* context);
    void CGIProcess(struct Context* context); //processinit, kevent(fd), 
    CGI();
    ~CGI();
};

bool isCGIRequest();//cgi 확인
#endif