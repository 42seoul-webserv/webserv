#include "ServerManager.hpp"

ServerManager::ServerManager(const std::string &configFilePath)
{
  ConfigParser parser; // TODO: Implement it
  _serverList = parser.parse(configFilePath);
}

ServerManager::~ServerManager()
{
}

void ServerManager::run()
{
  struct kevent event;
  // init kqueue
  if ((_kqueue = kqueue()) < 0)
    throw (std::runtime_error("Create Kqueue failed\n"));
  // init socket
  for (
    std::vector<Server>::iterator it = _serverList.begin();
    it != _serverList.end();
    ++it)
  {
    if ((it->_serverFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
      throw (std::runtime_error("Create Socket failed\n"));
    if (fcntl(it->_serverFD, F_SETFL, O_NONBLOCK) < 0) // control serverFD to non-block mode
      throw (std::runtime_error("fcntl non-block failed\n"));
    if (bind(it->_serverFD, reinterpret_cast<const sockaddr *>(&it->_socketAddr), sizeof(it->_socketAddr)) < 0)
      throw (std::runtime_error("Bind Socket failed\n"));
    if (listen(it->_serverFD, 10) < 0)
      throw (std::runtime_error("Listen Socket failed\n"));
    EV_SET(&event, it->_serverFD, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, NULL); // set event of serverFD
    if (kevent(_kqueue, &event, 1, NULL, 0, NULL) < 0) // event attach
      throw (std::runtime_error("Event attach failed\n"));
  }
  // RUN SERVER
  while (1)
  {
    // TODO: CHECK EVENTS IN KQUEUE
  }
}
