#include "ServerManager.hpp"

ServerManager::ServerManager(const std::string &configFilePath)
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
    attatchServerEvent(*server);
  }
}

// minkyeki, 11/19 추가.
// 서버 리스트를 돌면서 동일한 포트주소를 갖는지 체크, 동일하다면 해당 서버의 이름을 리턴.
std::string ServerManager::getServerName(in_port_t port_num) const
{
  std::vector<Server>::const_iterator itr = this->_serverList.begin();
  while (itr != _serverList.end())
  {
    if (itr->_socketAddr.sin_port == port_num) // if found target port
    {
      return (itr->_server_name);
    }
    itr++;
  }
  return ("webserv");
}

void ServerManager::attatchServerEvent(Server &server)
{
  // TODO: have to control leak?
  struct kevent events[1];
  struct Context *context = new struct Context(server._serverFD, server._socketAddr, acceptHandler, this);
  // Context { fd | socket_addr | handler | manager_ptr }

  EV_SET(&events[0], server._serverFD, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, context);
  // 등록하는 kevent
  if (kevent(_kqueue, events, 1, NULL, 0, NULL) < 0)
  {
    printLog("error: server: event attach failed\n", PRINT_RED);
    throw (std::runtime_error("Event attach failed\n"));
  }
  _contexts.push_back(context);
}
