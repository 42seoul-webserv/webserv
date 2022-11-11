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

class ServerManager
{
private:
    std::vector<Server> _serverList;
    FileDescriptor _kqueue;
public:
    FileDescriptor getKqueue() const;

public:
    ServerManager(const std::string& configFilePath);
    ~ServerManager();
    void run();
};

#endif //SERVERMANAGER_HPP
