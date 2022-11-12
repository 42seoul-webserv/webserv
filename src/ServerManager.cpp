#include "ServerManager.hpp"

ServerManager::ServerManager(const std::string &configFilePath)
{
  ConfigParser parser; // TODO: Implement it
  _serverList = parser.parse(configFilePath);
}

ServerManager::~ServerManager()
{
  for (
        std::vector<struct Context*>::iterator it = _contexts.begin();
        it != _contexts.begin();
        ++it
    )
  {
    delete (*it);
  }
}

void ServerManager::run()
{
  struct kevent event;

  if ((_kqueue = kqueue()) < 0)
    throw (std::runtime_error("Create Kqueue failed\n"));
  initServers();
  while (1)
  {
    int newEventCount = kevent(_kqueue, NULL, 0, &event, 1, NULL);

    if (newEventCount > 0 && (event.filter == EVFILT_READ || event.filter == EVFILT_WRITE))
    {
      struct Context* eventData = static_cast<struct Context*>(event.udata);
      try
      {
        eventData->handler(eventData);
      }
      catch (std::exception& e)
      {
        std::cerr << e.what();
      }
    }
  }
}

FileDescriptor ServerManager::getKqueue() const
{
  return _kqueue;
}

void ServerManager::initServers()
{
  for (
          std::vector<Server>::iterator server = _serverList.begin();
          server != _serverList.end();
          ++server)
  {
    server->openServer();
    attatchServerEvent(*server);
  }
}

void ServerManager::attatchServerEvent(Server &server)
{
  // TODO: have to control leak?
  struct kevent events[1];
  struct Context *context = new struct Context(server._serverFD, server._socketAddr, acceptHandler, this);

  EV_SET(&events[0], server._serverFD, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, context);
  if (kevent(_kqueue, events, 1, NULL, 0, NULL) < 0)
    throw (std::runtime_error("Event attach failed\n"));
  _contexts.push_back(context);
}
