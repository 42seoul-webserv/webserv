#include "ServerManager.hpp"

ServerManager::ServerManager(const std::string &configFilePath):
  _processor(*this)
{
  ConfigParser parser;
  _serverList = parser.parsing(configFilePath);
  //parser.displayAll();
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
  initServers(); // 여러 서버 세팅들을 모두 연다. (nginx config 참조)
  while (1)
  {
    // 서버 시작. 새 이벤트(Req)가 발생할 때 까지 무한루프. (감지하는 kevent)
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
    attachServerEvent(*server);
  }
}

std::string ServerManager::getServerName(in_port_t port_num) const
{
  std::vector<Server>::const_iterator itr = this->_serverList.begin();
  while (itr != _serverList.end())
  {
    if (itr->_socketAddr.sin_port == port_num) // if found target port
    {
      return (itr->_serverName);
    }
    itr++;
  }
  return ("webserv");
}

void ServerManager::attachServerEvent(Server &server)
{
  // TODO: have to control leak?
  struct kevent events[1];
  struct Context *context = new struct Context(server._serverFD, server._socketAddr, acceptHandler, this);
  // Context { fd | socket_addr | handler | manager_ptr }

  EV_SET(&events[0], server._serverFD, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, context);
  // 등록하는 kevent
  if (kevent(_kqueue, events, 1, NULL, 0, NULL) < 0)
  {
    printLog("error: server: event attachServerEvent failed\n", PRINT_RED);
    throw (std::runtime_error("Event attachServerEvent failed\n"));
  }
  _contexts.push_back(context);
}

std::vector<Server> &ServerManager::getServerList()
{
  return (_serverList);
}

Server& ServerManager::getMatchedServer(const HTTPRequest& req)
{
  for (
          std::vector<Server>::iterator it = _serverList.begin();
          it != _serverList.end();
          ++it
          )
  {
    Server& server = *it;
    std::string serverName = server._serverName + ':' + ft_itos(server._serverPort);
    std::string hostName = req._headers.at("host");
    if (hostName.find(':') == std::string::npos)
    {
      hostName += ":80";
    }
    if (hostName == serverName)
    {
      return (server);
    }
  }
  // no matched host, then check port
  for (
          std::vector<Server>::iterator it = _serverList.begin();
          it != _serverList.end();
          ++it
          )
  {
    Server& server = *it;
    std::string host = req._headers.at("host");
    std::string hostPort = host.substr(host.find(':') + 1);
    if (server._serverPort == ft_stoi(hostPort))
    {
      return (server);
    }
  }
  return (_serverList[0]);
}

void ServerManager::attachNewEvent(struct Context *context, const struct kevent &event)
{
  // 등록하는 kevent
  if (kevent(_kqueue, &event, 1, NULL, 0, NULL) < 0)
  {
    throw (std::runtime_error("Event attachNewEvent failed\n"));
  }
}
