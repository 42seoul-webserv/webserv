#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include <vector>
#include "Server.hpp"
#include "Parser.hpp"
#include "RequestProcessor.hpp"
#include "RequestParser.hpp"
#include "HTTPResponse.hpp"
#include "ThreadPool.hpp"
#include "CGI.hpp"
#include <sys/stat.h>
class ServerManager;

struct Context
{
    int fd;
    struct sockaddr_in addr;
    void (* handler)(struct Context* obj);
    ServerManager* manager;
    CGI* cgi;
    HTTPRequest* req;
    HTTPResponse* res; // -> for file FD, ContentLength... etc
    char* ioBuffer;
    size_t  bufferSize;
    size_t  totalIOSize; // 보낼 때 마다 합산.
    FileDescriptor threadKQ;
    std::vector<struct Context*>* connectContexts;

    Context(){}
    Context(int _fd,
            struct sockaddr_in _addr,
            void (* _handler)(struct Context* obj),
            ServerManager* _manager) :
            fd(_fd),
            addr(_addr),
            handler(_handler),
            manager(_manager),
            cgi(NULL),
            req(NULL),
            res(NULL),
            ioBuffer(NULL),
            bufferSize(0),
            totalIOSize(0),
            threadKQ(0),
            connectContexts(NULL)
    {
    }
    ~Context()
    {
      if (connectContexts)
      {
        for (
              std::vector<struct Context*>::iterator it = connectContexts->begin();
              it != connectContexts->end();
              ++it
              )
        {
          struct Context* context = *it;

          if (context->ioBuffer != NULL)
            delete (context->ioBuffer);
          // 이렇게 안하면 재귀 호출됨...
          if (context != this)
            free(context);
        }
        // 중복되는 자료.
        delete (this->connectContexts);
      }
    }
};

class RequestParser;

class ServerManager
{
private:
    std::vector<Server> _serverList;
    std::vector<struct Context*> _contexts;
    FileDescriptor _kqueue;
    RequestProcessor _processor;
    RequestParser _requestParser;
    ThreadPool _threadPool;
public:
    explicit ServerManager(const std::string& configFilePath);
    ~ServerManager();
    void run();
    void initServers();
    void attachServerEvent(Server& server);
    int attachNewEvent(struct Context* context, const struct kevent& event);
    FileDescriptor getKqueue() const;
    std::string getServerName(in_port_t port_num) const;
    std::vector<Server>& getServerList();
    RequestProcessor& getRequestProcessor();
    RequestParser& getRequestParser();
    Server& getMatchedServer(const HTTPRequest& req);
};

void socketReceiveHandler(struct Context* context);
void acceptHandler(struct Context* context);
void handleEvent(struct kevent* event);
void writeFileHandle(struct Context* context);
void CGIWriteHandler(struct Context* context);
void clearContexts(struct Context* context);
void CGIChildHandler(struct Context* context);
#endif //SERVERMANAGER_HPP
