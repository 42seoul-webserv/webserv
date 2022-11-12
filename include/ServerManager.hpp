#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include <vector>
#include <Server.hpp>
//#include <ConfigParser.hpp>

class ConfigParser
{
public:
    std::vector<Server> parse(const std::string& configFilePath)
    {
      std::vector<Server> serverList;
      Server newServer;

      (void) configFilePath;
      inet_pton(AF_INET, "0.0.0.0", &newServer._socketAddr.sin_addr);
      newServer._socketAddr.sin_port = ntohs(42424);
      newServer._socketAddr.sin_family = AF_INET;
      newServer._index = "index.html";
      newServer._root = "./www";
      serverList.push_back(newServer);
      return (serverList);
    }
};

struct Context;

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
      manager(_manager)
    {
    }
};

void readHandler(struct Context *context);
void acceptHandler(struct Context *context);
void responseHandler(struct Context *context);

#endif //SERVERMANAGER_HPP
