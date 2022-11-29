#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include <vector>
#include "Server.hpp"
#include "Parser.hpp"
#include "RequestProcessor.hpp"
#include "RequestParser.hpp"
#include "CGI.hpp"
#include "ThreadPool.hpp"

class ServerManager;

struct Context
{
    int fd;
    struct sockaddr_in addr;
    void (* handler)(struct Context* obj);
    ServerManager* manager;
    HTTPRequest* req;
    CGI* cgi;
    HTTPResponse* res; // -> for file FD, ContentLength... etc
    char* read_buffer;
    size_t  buffer_size;
    size_t  total_read_size; // 보낼 때 마다 합산.
    FileDescriptor threadKQ;

    Context(int _fd,
            struct sockaddr_in _addr,
            void (* _handler)(struct Context* obj),
            ServerManager* _manager) :
            fd(_fd),
            addr(_addr),
            handler(_handler),
            manager(_manager),
            req(NULL),
            res(NULL),
            cgi(NULL),
            read_buffer(NULL),
            buffer_size(0),
            total_read_size(0),
            threadKQ(0)

    {
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
    static void attachNewEvent(struct Context* context, const struct kevent& event);
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
void pipeWriteHandler(struct Context* context);

#endif //SERVERMANAGER_HPP
