#include "ServerManager.hpp"
#include "RequestProcessor.hpp"
#include <sstream>

void printLog(const std::string& log, const std::string& color = PRINT_RESET)
{
  std::cout << color << log << PRINT_RESET;
}

MethodType getMethodType(const std::string& method)
{
  const char* const METHODS[] = {"GET", "POST", "PUT", "PATCH", "DELETE"};
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

std::string getClientIP(const struct sockaddr_in* addr)
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

void socketReceiveHandler(struct Context* context)
{
  if (!context)
    throw (std::runtime_error("NULL context"));
  context->manager->getRequestParser().parseRequest(context);
}

// TODO : client session time?
void acceptHandler(struct Context* context)
{
  socklen_t len = sizeof(context->addr);
  FileDescriptor newSocket;

  if ((newSocket = accept(context->fd, reinterpret_cast<sockaddr*>(&context->addr), &len)) < 0)
  {
    printLog("error:" + getClientIP(&context->addr) + " : accept failed\n", PRINT_RED);
    throw (std::runtime_error("Accept failed\n"));
  }
  else
  {
    printLog(getClientIP(&context->addr) + " : connect\n", PRINT_GREEN);
    struct Context* newContext = new struct Context(newSocket, context->addr, readHandler, context->manager);
    struct kevent event;
    EV_SET(&event, newSocket, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, newContext);
    if (kevent(context->manager->getKqueue(), &event, 1, NULL, 0, NULL) < 0)
    {
      printLog("error: client: " + getClientIP(&context->addr) + " : event attachServerEvent failed\n", PRINT_RED);
      throw (std::runtime_error("Event attachServerEvent failed (read)\n"));
    }
  }
}

void handleEvent(struct kevent* event)
{
  struct Context* eventData = static_cast<struct Context*>(event->udata);
  try
  {
    eventData->handler(eventData);
  }
  catch (std::exception& e)
  {
    printLog(e.what(), PRINT_RED);
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

std::string ft_itos(int i)
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