#include "ServerManager.hpp"


ServerManager::ServerManager(const std::string &configFilePath)
{
  ConfigParser parser; // TODO: Implement it
  _serverList = parser.parse(configFilePath);
}

ServerManager::~ServerManager()
{
}

struct Context
{
    int fd;
    struct sockaddr_in addr;
    void (*handler)(struct Context *obj);
    ServerManager* manager;
};

// TODO: THIS FUNCTION IS TEMPORARY
#include <sstream>
static std::string getResponse(FileDescriptor indexFile)
{
  char buffer[1024] = {0};
  int readSize = read(indexFile, buffer, sizeof(buffer)); // blocked
  std::ostringstream ss;
  std::string result;
  ss << readSize;

  result += "HTTP/1.1 200 OK\r\n";
  result += "Server: webserv\r\n";
  result += "Content-length: " + ss.str() + "\r\n";
  result += "Content-Type: text/html\r\n";
  result += "\r\n";
  result += buffer;

  return (result);
}

void responseHandler(struct Context *obj)
{
  FileDescriptor indexFile ;

  if ((indexFile = open("../index.html", O_RDONLY)) < 0)
    throw (std::runtime_error("OPEN FAILED\n"));

  std::string res = getResponse(indexFile);
  if (send(obj->fd, res.data(), res.size(), 0) < 0)
    throw (std::runtime_error("Send Failed\n"));
  close(obj->fd);
  delete (obj);
}

void readHandler(struct Context *obj)
{
  // read socket
  char buffer[1024] = {0};
  struct kevent event;

  if (read(obj->fd, buffer, sizeof(buffer)) < 0)
    std::cerr << "READ ERR\n";
  // attach response event
  struct Context* resObj = new struct Context;
  resObj->fd = obj->fd;
  resObj->addr = obj->addr;
  resObj->manager = obj->manager;
  resObj->handler = responseHandler;
  EV_SET(&event, resObj->fd, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, resObj); // set events of serverFD [read]
  if (kevent(obj->manager->getKqueue(), &event, 1, NULL, 0, NULL) < 0) // events attach
    throw (std::runtime_error("Event attach failed (response)\n"));
  delete (obj);
}

static std::string getClientIP(struct sockaddr_in* addr)
{
  char str[INET_ADDRSTRLEN];
  struct sockaddr_in* pV4Addr = addr;
  struct in_addr ipAddr = pV4Addr->sin_addr;
  inet_ntop( AF_INET, &ipAddr, str, INET_ADDRSTRLEN );

  return (str);
}

void acceptHandler(struct Context *obj)
{
  socklen_t len = sizeof(obj->addr);
  FileDescriptor newSocket;
  struct Context* readObj = new struct Context;
  struct kevent event;

  if ((newSocket = accept(obj->fd, reinterpret_cast<sockaddr *>(&obj->addr), &len)) < 0)
    throw (std::runtime_error("Accept failed\n"));
  std::cout << "connect with : " << getClientIP(&obj->addr) << "\n";
  // attach read event
  readObj->fd = newSocket;
  readObj->addr = obj->addr;
  readObj->manager = obj->manager;
  readObj->handler = readHandler;
  EV_SET(&event, newSocket, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, readObj); // set events of serverFD [read]
  if (kevent(obj->manager->getKqueue(), &event, 1, NULL, 0, NULL) < 0) // events attach
    throw (std::runtime_error("Event attach failed (read)\n"));
}

void ServerManager::run()
{
  struct kevent events[2];
  struct Context obj = {};

  // init kqueue
  if ((_kqueue = kqueue()) < 0)
    throw (std::runtime_error("Create Kqueue failed\n"));
  // init socket -> have to make function
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
    obj.fd = it->_serverFD;
    obj.addr = it->_socketAddr;
    obj.handler = acceptHandler;
    obj.manager = this;
    EV_SET(&events[0], it->_serverFD, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, &obj); // set events of serverFD [read]
    EV_SET(&events[1], it->_serverFD, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, &obj); // set events of serverFD [write]
    if (kevent(_kqueue, events, 2, NULL, 0, NULL) < 0) // events attach
      throw (std::runtime_error("Event attach failed\n"));
  }
  // RUN SERVER
  while (1)
  {
    // TODO: CHECK EVENTS IN KQUEUE
    int n = kevent(_kqueue, NULL, 0, events, 1, NULL);
    if (n <= 0)
      continue ;
    else if ((events[0].filter == EVFILT_READ || events[0].filter == EVFILT_WRITE))
    {
      struct Context* data = static_cast<struct Context*>(events[0].udata);
      try
      {
        data->handler(data);
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
