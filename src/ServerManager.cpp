#include "ServerManager.hpp"
#include "ThreadPool.hpp"
#include <cstring>

ServerManager::ServerManager(const std::string& configFilePath) :
        _processor(*this),
        _threadPool(THREAD_NO)
{
  ConfigParser parser;
  _serverList = parser.parseConfigFile(configFilePath);
//  parser.displayAll();
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
    *it = NULL;
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
  if (THREAD_MODE)
  {
    _threadPool._serverKQ = _kqueue;
    _threadPool.createPool();
  }
  while (1)
  {
    // 서버 시작. 새 이벤트(Req)가 발생할 때 까지 무한루프. (감지하는 kevent)
    int newEventCount = kevent(_kqueue, NULL, 0, &event, 1, NULL);

    if (newEventCount == -1)
    { // nothing happen
      printLog("EV ERR (-1)\n", PRINT_RED);
      continue;
    }
    else if (newEventCount == 0)
    { // time limit expired -> never happen
      printLog("time limit expired\n", PRINT_BLUE);
    }
    else if (event.filter == EVFILT_READ \
            || event.filter == EVFILT_WRITE \
            || event.filter == EVFILT_PROC)
    {
      if (THREAD_MODE)
      {
        _threadPool.attachNewEvent(&event);
      }
      else
        handleEvent(&event);
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
          ++server
          )
  {
    server->openServer();
    for (
        std::vector<Server>::iterator temp = _serverList.begin();
        temp != server;
        ++temp
        )
    {
      if ((*temp)._serverPort == (*server)._serverPort && (*temp)._serverName == (*server)._serverName)
        throw (std::runtime_error("Same port and Same server\n"));
      if ((*temp)._serverPort == (*server)._serverPort)
        return ;
    }
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
  return (DEFAULT_SERVER_NAME);
}

void ServerManager::attachServerEvent(Server& server)
{
  // TODO: have to control leak?
  struct kevent events[1];
  struct Context* context = new struct Context(server._serverFD, server._socketAddr, acceptHandler, this);
  context->threadKQ = _kqueue;

  EV_SET(&events[0], server._serverFD, EVFILT_READ, EV_ADD, 0, 0, context);
  // 등록하는 kevent
  if (kevent(_kqueue, events, 1, NULL, 0, NULL) < 0)
  {
    printLog("error: server: event attachServerEvent failed\n", PRINT_RED);
    throw (std::runtime_error("Event attachServerEvent failed\n"));
  }
  _contexts.push_back(context);
}

std::vector<Server>& ServerManager::getServerList()
{
  return (_serverList);
}

RequestProcessor& ServerManager::getRequestProcessor()
{
  return (_processor);
}
RequestParser& ServerManager::getRequestParser()
{
  return (_requestParser);
}

Server& ServerManager::getMatchedServer(const HTTPRequest& req)
{
  std::map<std::string,std::string>::const_iterator mit;
  for (
          std::vector<Server>::iterator it = _serverList.begin();
          it != _serverList.end();
          ++it
          )
  {
    Server& server = *it;
    std::string serverName = server._serverName + ':' + ft_itos(server._serverPort);
    mit = req.headers.find("Host");
    if (mit == req.headers.end())
    {
      continue;
    }
    std::string host;
    host.assign(mit->second);
    if (host.find(':') == std::string::npos)
    {
      host += ":80";
    }
    if (host == serverName)
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
    mit = req.headers.find("Host");
    if (mit == req.headers.end())
    {
      continue;
    }
    std::string host;
    host.assign(mit->second);
    std::string hostPort = host.substr(host.find(':') + 1);
    if (server._serverPort == ft_stoi(hostPort))
    {
      return (server);
    }
  }
  return (_serverList[0]);
}

int ServerManager::attachNewEvent(struct Context* context, const struct kevent& event)
{
  FileDescriptor kq;

  if (THREAD_MODE)
    kq = context->threadKQ;
  else
    kq = _kqueue;
  if (kevent(kq, &event, 1, NULL, 0, NULL) < 0)
  {
    if (DEBUG_MODE)
    {
      printLog("event attach failed\n", PRINT_YELLOW);
      std::cout << kq << " (attach) \n";
      std::cout << event.ident << "(ident) (attach) \n";
      std::cout << strerror(errno) << " (attach) \n";
      std::cout << context->threadKQ << " (attach) \n";
      std::cout << context->req->status << " (attach) \n";
    }
    return (FAILED);
  }
  return (0);
}
