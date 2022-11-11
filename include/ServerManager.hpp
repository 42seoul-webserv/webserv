#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include <vector>
#include <Server.hpp>
#include <ConfigParser.hpp>

class ServerManager
{
private:
    std::vector<Server> _serverList;
    FileDescriptor _kqueue;
public:
    ServerManager(const std::string& configFilePath);
    ~ServerManager();
    void run();
};

#endif //SERVERMANAGER_HPP
