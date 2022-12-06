#include "ServerManager.hpp"
#include "RequestProcessor.hpp"
#include "HTTPResponse.hpp"
#include "CGI.hpp"
#include <sstream>
#include <sys/stat.h>

void printLog(const std::string& log, const std::string& color = PRINT_RESET)
{
  std::cout << color << log << PRINT_RESET;
}

MethodType getMethodType(const std::string& method)
{
  const char* const METHODS[] = {"GET", "POST", "PUT", "PATCH", "DELETE", "HEAD"};
  const size_t NUMBER_OF_METHOD = sizeof(METHODS) / sizeof(char*);

  for (
          size_t i = 0; i < NUMBER_OF_METHOD; ++i
          )
  {
    if (method == METHODS[i])
    {
      return (MethodType(i));
    }
  }
  return (UNDEFINED);
}

std::string methodToString(const MethodType method)
{
  const char* const METHODS[] = {"GET", "POST", "PUT", "PATCH", "DELETE", "HEAD"};

  if (method == UNDEFINED)
    return ("UNDEFINED");
  else
    return (METHODS[(int)method]);
}

std::string getClientIP(struct sockaddr_in* addr)
{
  char str[INET_ADDRSTRLEN];
  const struct sockaddr_in* pV4Addr = addr;
  struct in_addr ipAddr = pV4Addr->sin_addr;
  inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);

  return (str);
}

void CGIChildHandler(struct Context* context)
{
  waitpid(context->cgi->pid, &context->cgi->exitStatus, 0);
/*  struct kevent event;
  EV_SET(&event, context->cgi->pid, EVFILT_PROC, EV_DELETE, 0, 0,NULL);
  context->manager->attachNewEvent(context, event);*/
  unlink(context->cgi->writeFilePath.c_str());
  struct Context* origin = (*(context->connectContexts))[0];
  if (context->cgi->exitStatus)
  {
    close(context->cgi->readFD);
    HTTPResponse* response = new HTTPResponse(ST_BAD_GATEWAY, "gateway broken", context->manager->getServerName(context->addr.sin_port));
    response->setFd(-1);
    response->addHeader(HTTPResponseHeader::CONTENT_LENGTH(0));
    delete context->cgi;
    context->cgi = NULL;
    response->sendToClient(context);
  }
  else
  {
    //context->connectContexts->push_back(context);
    //context->req = new HTTPRequest(*context->req);
    free(context);
    origin->cgi->parseCGI(origin);
    
    delete origin->cgi;//소멸자로 fd unlink해주는게 좋을듯?
    origin->cgi = NULL;
    origin->res->sendToClient(origin);
  }
}

void CGIWriteHandler(struct Context* context)
{
  HTTPRequest& req = *context->req;

  ssize_t writeSize = 0;
  if ((writeSize = write(context->cgi->writeFD, &req.body.c_str()[context->totalIOSize], req.body.size() - context->totalIOSize)) < 0)
  {
    printLog("error\t\t" + getClientIP(&context->addr) + "\t: write failed\n", PRINT_RED);
  }
  context->totalIOSize += writeSize; // get total write size
  if (context->totalIOSize >= req.body.size()) // If write finished
  {
    close(context->cgi->writeFD);
    context->cgi->CGIChildEvent(context);
    free (context);
  }
}

void socketReceiveHandler(struct Context* context)
{
	if (DEBUG_MODE)
		printLog("sk recv handler called\n", PRINT_CYAN);
  if (!context)
    throw (std::runtime_error("NULL context"));
  context->manager->getRequestParser().parseRequest(context);
}

// TODO : client session time?
void acceptHandler(struct Context* context)
{
//  static uint32_t connections;
  if (DEBUG_MODE)
    printLog("accept handler called\n", PRINT_CYAN);
  socklen_t len = sizeof(context->addr);
  FileDescriptor newSocket;

  if ((newSocket = accept(context->fd, reinterpret_cast<sockaddr*>(&context->addr), &len)) < 0)
  {
    if (DEBUG_MODE)
      printLog("error:" + getClientIP(&context->addr) + " : accept failed\n", PRINT_RED);
    if (THREAD_MODE)
      delete (context);
    return ;
  }
  else
  {
    // set socket option
    if (fcntl(newSocket, F_SETFL, O_NONBLOCK) < 0)
    {
      throw (std::runtime_error("fcntl non block failed\n"));
    }
    struct linger optLinger = {1, 0};
    if (setsockopt(newSocket, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger)) < 0 )
    {
      throw (std::runtime_error("Socket opt failed\n"));
    }
    printLog("connect\t\t" + getClientIP(&context->addr) + "\n" , PRINT_GREEN);

    struct Context* newContext = new struct Context(newSocket, context->addr, socketReceiveHandler, context->manager);
    newContext->fd = newSocket;
    newContext->threadKQ = context->threadKQ;
    newContext->connectContexts = new std::vector<struct Context*>();
    newContext->connectContexts->push_back(newContext);
    struct kevent event;
    EV_SET(&event, newSocket, EVFILT_READ, EV_ADD, 0, 0, newContext);
    context->manager->attachNewEvent(context, event);
    if (THREAD_MODE)
      delete (context);
  }
}

void handleEvent(struct kevent* event)
{
  struct Context* eventData = static_cast<struct Context*>(event->udata);
  try
  {
    if (event->filter != EVFILT_PROC && (event->flags & EV_EOF || event->fflags & EV_EOF))
    {
      printLog("Client closed connection : " + getClientIP(&eventData->addr) + "\n", PRINT_YELLOW);
      shutdown(eventData->fd, SHUT_RDWR);
      close(eventData->fd);
      clearContexts(eventData);
      delete (eventData);
    }
    else if (event->flags & EV_ERROR)
    {
      printLog("EV ERROR case\n", PRINT_YELLOW);
      shutdown(eventData->fd, SHUT_RDWR);
      close(eventData->fd);
      if (eventData->req != NULL)
        delete (eventData->req);
      eventData->req = NULL;
      if (eventData->res != NULL)
        delete (eventData->res);
      eventData->res = NULL;
      delete (eventData);
    }
    else
    {
      eventData->handler(eventData);
    }
  }
  catch (std::exception& e)
  {
    printLog(e.what(), PRINT_RED);
  }
}

// nonblocking write.
void writeFileHandle(struct Context* context)
{
  HTTPResponse& res = *context->res;
  HTTPRequest& req = *context->req;

  ssize_t writeSize = 0;
  if ((writeSize = write(context->fd, &req.body.c_str()[context->totalIOSize], req.body.size() - context->totalIOSize)) < 0)
  {
    printLog("error\t\t" + getClientIP(&context->addr) + "\t: write failed\n", PRINT_RED);
  }
  context->totalIOSize += writeSize; // get total write size
  if (context->totalIOSize >= req.body.size()) // If write finished
  {
    close(context->fd);
    free(context);
  }
}

//https://stackoverflow.com/questions/154536/encode-decode-urls-in-c
std::string encodePercentEncoding(const std::string& str)
{
  std::ostringstream escaped;
  escaped.fill('0');
  escaped << std::hex;

  for (
          std::string::const_iterator i = str.begin(), n = str.end(); i != n; ++i
          )
  {
    std::string::value_type c = (*i);
    // Keep alphanumeric and other accepted characters intact
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
    {
      escaped << c;
      continue;
    }
    // Any other characters are percent-encoded
    escaped << std::uppercase;
    escaped << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
    escaped << std::nouppercase;
  }
  return (escaped.str());
}

std::string decodePercentEncoding(const std::string& str)
{
  std::string result;

  for (
          std::string::const_iterator i = str.begin(), n = str.end(); i != n; ++i
          )
  {
    std::string::value_type c = (*i);

    if (c == '%')
    {
      std::stringstream ss;
      int byte;
      char hexCode[3] = {*++i, *++i, 0};

      ss << std::hex << hexCode;
      ss >> byte;
      result += static_cast<char>(byte);
    }
    else
    {
      result += c;
    }
  }
  return (result);
}

std::string ft_itos(ssize_t i)
{
  std::stringstream ss;
  ss << i;
  return (ss.str());
}

int ft_stoi(const std::string& str)
{
  int res;
  std::stringstream ss;

  ss << str;
  ss >> res;
  return (res);
}

long FdGetFileSize(int fd)
{
  if (fd < 0)
    return 0;
  struct stat stat_buf;
  int rc = fstat(fd, &stat_buf);
  return rc == 0 ? stat_buf.st_size : -1;
}

void clearContexts(struct Context* context)
{
  // 내부에서 본인 제외 모두 삭제.
  HTTPResponse* res = NULL;
  HTTPRequest* req = NULL;

  for (
          std::vector<struct Context*>::iterator it = context->connectContexts->begin();
          it != context->connectContexts->end();
          ++it
          )
  {
    struct Context* data = *it;

    if (data == context)
    {
      continue;
    }
    if (data->req)
    {
      req = data->req;
      data->req = NULL;
    }
    if (data->res)
    {
      res = data->res;
      data->res = NULL;
    }
    if (data->ioBuffer != NULL)
    {
      delete (data->ioBuffer);
      data->ioBuffer = NULL;
    }
    if (data != context)
    {
      free (data);
      data = NULL;
    }
  }
  if (res != NULL)
  {
    delete (res);
    res = NULL;
  }
  if (req != NULL)
  {
    delete (req);
    req = NULL;
  }
  context->connectContexts->clear();
  context->connectContexts->push_back(context);
}
