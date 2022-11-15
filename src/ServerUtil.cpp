#include "ServerManager.hpp"

MethodType getMethodType(const std::string& method)
{
  const char *const METHODS[] = {"GET", "POST", "PUT", "PATCH", "DELETE"};
  const size_t NUMBER_OF_METHOD = sizeof(METHODS) / sizeof(char*);

  for (size_t i = 0; i < NUMBER_OF_METHOD; ++i)
  {
    if (method == METHODS[i])
      return (MethodType(i));
  }
  return (UNDEFINED);
}

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

// client IP is defined when accept successed...
// TODO : separate to another file
static std::string getClientIP(struct sockaddr_in* addr)
{
  char str[INET_ADDRSTRLEN];
  struct sockaddr_in* pV4Addr = addr;
  struct in_addr ipAddr = pV4Addr->sin_addr;
  inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);

  return (str);
}

// TODO: Handlers -> ServerManager Methods or something else...
void responseHandler(struct Context *context)
{
  FileDescriptor indexFile ;

  if ((indexFile = open("../index.html", O_RDONLY)) < 0)
    throw (std::runtime_error("OPEN FAILED\n"));
  std::string res = getResponse(indexFile);
  if (send(context->fd, res.data(), res.size(), 0) < 0)
    throw (std::runtime_error("Send Failed\n"));
  close(context->fd);
  delete (context);
}

void readHandler(struct Context *context)
{
  // read socket TODO: if chunked..?
  char buffer[1024] = {0};
  struct kevent event;

  // TODO: get HTTPRequest Object from HTTPRequestParser
  if (read(context->fd, buffer, sizeof(buffer)) < 0)
    std::cerr << "READ ERR\n";

  // FIXME: below codes only can do GET METHOD
  // attach response event
  {
    struct Context* newContext = new struct Context(context->fd, context->addr, responseHandler, context->manager);
    EV_SET(&event, newContext->fd, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, newContext);
    if (kevent(context->manager->getKqueue(), &event, 1, NULL, 0, NULL) < 0)
      throw (std::runtime_error("Event attach failed (response)\n"));
    delete (context);
  }
}

void acceptHandler(struct Context *context)
{
  socklen_t len = sizeof(context->addr);
  FileDescriptor newSocket;

  if ((newSocket = accept(context->fd, reinterpret_cast<sockaddr *>(&context->addr), &len)) < 0)
    throw (std::runtime_error("Accept failed\n"));
  std::cout << "connect with : " << getClientIP(&context->addr) << "\n";

  // attach read event
  {
    struct Context* newContext = new struct Context(newSocket, context->addr, readHandler, context->manager);
    struct kevent event;

    EV_SET(&event, newSocket, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, newContext);
    if (kevent(context->manager->getKqueue(), &event, 1, NULL, 0, NULL) < 0)
      throw (std::runtime_error("Event attach failed (read)\n"));
  }
}
