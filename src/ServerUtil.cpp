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

// TODO: Handlers -> ServerManager Methods or something else...
void responseHandler(struct Context* context)
{
  FileDescriptor indexFile;

  if ((indexFile = open("../_index.html", O_RDONLY)) < 0)
  {
    printLog("error: client: " + getClientIP(&context->addr) + " : open failed\n", PRINT_RED);
    throw (std::runtime_error("Open Failed\n"));
  }
  std::string res = getResponse(indexFile);
  if (send(context->fd, res.data(), res.size(), 0) < 0)
  {
    printLog("error: client: " + getClientIP(&context->addr) + " : send failed\n", PRINT_RED);
    throw (std::runtime_error("Send Failed\n"));
  }
  printLog(getClientIP(&context->addr) + " send response\n", PRINT_BLUE);
  close(context->fd);
  close(indexFile);

  delete (context);
}

void readHandler(struct Context* context)
{
  // read socket TODO: if chunked..?
  char buffer[BUFFER_SIZE] = {0};
  struct kevent event;

  // TODO: get HTTPRequest Object from HTTPRequestParser
  // 해당 부분이 non-blocked 형식으로 하는데, 따로 kevent로 돌려야하나? - request handling 어떻게함?
  // 큰 파일이 들어오는 경우 header만 먼저 읽어들이고 이후의 데이터를 판단해야할 것.
  if (recv(context->fd, buffer, sizeof(buffer), MSG_DONTWAIT) < 0)
  {
    printLog("error: " + getClientIP(&context->addr) + " : receive failed\n", PRINT_RED);
    throw (std::runtime_error("receive failed\n"));
  }
  else
  {
    // FIXME: below codes only can do GET METHOD
    struct Context* newContext = new struct Context(context->fd, context->addr, responseHandler, context->manager);
    EV_SET(&event, newContext->fd, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, newContext);
    if (kevent(context->manager->getKqueue(), &event, 1, NULL, 0, NULL) < 0)
    {
      printLog("error: " + getClientIP(&context->addr) + " : event attachServerEvent failed\n", PRINT_RED);
      throw (std::runtime_error("Event attachServerEvent failed (response)\n"));
    }
    delete (context);
  }
}

void CGIChildHandler(struct Context* context)
{
  waitpid(context->cgi->pid, &context->cgi->exitStatus, 0);
  if (context->cgi->exitStatus)
  {
    close(context->cgi->readFD);
    HTTPResponse* response = new HTTPResponse(ST_BAD_GATEWAY, "gateway borken", context->manager->getServerName(context->addr.sin_port));
    response->setFd(-1);
    response->addHeader(HTTPResponseHeader::CONTENT_LENGTH(0));
    delete context->cgi;
    context->cgi = NULL;
    response->sendToClient(context);
  }
  else
  {
    CGI::parseCGI(context);
    delete context->cgi;
    context->cgi = NULL;
    char buffer[BUFFER_SIZE];
    buffer[BUFFER_SIZE - 1] = '\0';
    read(context->res->getFd(), buffer, BUFFER_SIZE);
    std::cout << "fd body chekc"<< buffer << std::endl;
    context->res->sendToClient(context);
  }
}

void pipeWriteHandler(struct Context* context)
{
  size_t count;
  char buffer[BUFFER_SIZE] = {0};

  count = context->req->body.copy(buffer, BUFFER_SIZE);
  context->req->body.erase(0, count);
  if (count != 0)//re wirte
  {
    std::cout << "write check : "<<write(context->cgi->writeFD, buffer, count) << std::endl;
      //throw std::runtime_error("write failed");
  }
  else//pid kevent register, cgichildHandler call
  {
    struct Context* newContext = new struct Context(context->fd, context->addr, CGIChildHandler, context->manager);
    newContext->cgi = context->cgi;
    newContext->req = context->req;
    struct kevent event;
    EV_SET(&event, context->cgi->pid, EVFILT_PROC, EV_ADD | EV_ENABLE, NOTE_EXIT | NOTE_EXITSTATUS, context->cgi->exitStatus, newContext);
    context->manager->attachNewEvent(newContext, event);
    close(context->cgi->writeFD);
    delete (context);
  }
}

void socketReceiveHandler(struct Context* context)
{
//  printLog("sk recv handler called\n", PRINT_CYAN);
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
    return ;
  }
  else
  {
    // set socket option
    if (fcntl(newSocket, F_SETFL, O_NONBLOCK) < 0)
    {
      throw (std::runtime_error("fcntl non block failed\n"));
    }
    struct linger _linger = {1, 0};
    if (setsockopt(newSocket, SOL_SOCKET, SO_LINGER, &_linger, sizeof(_linger)) < 0 )
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
    if (context->req != NULL)
    {
      delete (context->req);
    }
    if (context->res != NULL)
    {
      delete (context->res);
    }
    delete (context);
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

  for (
          std::vector<struct Context*>::iterator it = context->connectContexts->begin();
          it != context->connectContexts->end();
          ++it
          )
  {
    struct Context* data = *it;

    if (data == context)
      continue;

    if (data->req != NULL)
    {
      delete (data->req);
      data->req = NULL;
    }
    if (data->res)
      res = data->res;
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
  context->connectContexts->clear();
  context->connectContexts->push_back(context);
}