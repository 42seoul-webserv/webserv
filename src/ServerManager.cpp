#include "ServerManager.hpp"

ServerManager::ServerManager(const std::string &configFilePath)
{
  ConfigParser parser;
  _serverList = parser.parsing(configFilePath);
  parser.displayAll();
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
  {
    printLog("error: server: create kqueue failed\n", PRINT_RED);
    exit(1);
  }
  initServers();
  while (1)
  {
    int newEventCount = kevent(_kqueue, NULL, 0, &event, 1, NULL);

    if (newEventCount == -1) // nothing happen
      continue ;
    else if (newEventCount == 0) // time limit expired -> never happen
      printLog("time limit expired\n", PRINT_BLUE);
    else if (event.filter == EVFILT_READ || event.filter == EVFILT_WRITE)
      handleEvent(&event);
    else if (event.filter == EV_ERROR) // request 도중에 error 가 난 상황이라면?
      printLog("Error: EV_ERROR\n", PRINT_RED);
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
  {
    printLog("error: server: event attach failed\n", PRINT_RED);
    throw (std::runtime_error("Event attach failed\n"));
  }
  _contexts.push_back(context);
}
