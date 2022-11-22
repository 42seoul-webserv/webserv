#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include <vector>
#include <Server.hpp>
#include <Parser.hpp>
#include <stdexcept>
struct Context;
struct HTTPrequest;

class ServerManager
{
private:
    std::vector<Server> _serverList;
    std::vector<struct Context *> _contexts;
    FileDescriptor _kqueue;
public:
    ServerManager(const std::string& configFilePath);
    ~ServerManager();
    void run();
    void initServers();
    void attatchServerEvent(Server& server);
    FileDescriptor getKqueue() const;

};

struct Context
{
    int fd;
    struct sockaddr_in addr;
    void (*handler)(struct Context *obj);
    ServerManager* manager;
    Context(int _fd,
            struct sockaddr_in _addr,
            void (*_handler)(struct Context *obj),
            ServerManager* _manager):
      fd(_fd),
      addr(_addr),
      handler(_handler),
      manager(_manager),
      _request(NULL)
    {
    }
    HTTPrequest* _request;
};

void readHandler(struct Context *context);
void acceptHandler(struct Context *context);
void responseHandler(struct Context *context);
void handleEvent(struct kevent *event);

#endif //SERVERMANAGER_HPP
