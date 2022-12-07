#include "HTTPResponse.hpp"
#include "ServerManager.hpp"

/**----------------------
 * * HeaderType         |
 *----------------------*/
std::string HeaderType::GET_DAY(long tm_wday)
{
  switch (tm_wday)
  {
    case 0:
      return ("Sun");
    case 1:
      return ("Mon");
    case 2:
      return ("Tue");
    case 3:
      return ("Wed");
    case 4:
      return ("Thu");
    case 5:
      return ("Fri");
    case 6:
      return ("Sat");
    default:
      return ("null");
  }
}

std::string HeaderType::GET_MON(long tm_wmon)
{
  switch (tm_wmon)
  {
    case 0:
      return ("Jan");
    case 1:
      return ("Feb");
    case 2:
      return ("Mar");
    case 3:
      return ("Apr");
    case 4:
      return ("May");
    case 5:
      return ("Jun");
    case 6:
      return ("Jul");
    case 7:
      return ("Aug");
    case 8:
      return ("Sep");
    case 9:
      return ("Oct");
    case 10:
      return ("Nov");
    case 11:
      return ("Dec");
    default:
      return ("null");
  }
}

std::string ft_itos_width(int num, int minimum_width)
{
  std::string origin = ft_itos(num);
  std::string result;
  if (origin.size() < minimum_width)
  {
    for (int i = 0; i < minimum_width - origin.size(); i++)
    {
      result.append("0");
    }
    result.append(origin);
    return (result);
  }
  else
    return (origin);
}

std::string HeaderType::getDate()
{
  time_t curTime = time(NULL);          // get current time info
  struct tm* pLocal = gmtime(&curTime); // convert to struct for easy use
  if (pLocal == NULL)
  {
    // ...
    return ("null");
  }
  std::string result;
  result = GET_DAY(pLocal->tm_wday) + ", " + ft_itos_width(pLocal->tm_mday, 2) + " " + GET_MON(pLocal->tm_mon) + " " + ft_itos(pLocal->tm_year + 1900) + " " +
           ft_itos_width(pLocal->tm_hour, 2) + ":" + ft_itos_width(pLocal->tm_min, 2) + ":" + ft_itos_width(pLocal->tm_sec, 2) + " GMT";
  return (result);
}

// ! WARN: need to handle [Date + diff --> next day / previous day] ...etc
std::string HeaderType::getDateByYearOffset(int year_diff)
{
  time_t curTime = time(NULL);          // get current time info
  struct tm* pLocal = gmtime(&curTime); // convert to struct for easy use
  if (pLocal == NULL)
  {
    // ...
    return ("null");
  }
  std::string result;
  result = GET_DAY(pLocal->tm_wday) + ", " + ft_itos_width(pLocal->tm_mday, 2) + " " + GET_MON(pLocal->tm_mon) + " " + ft_itos(pLocal->tm_year + 1900 + year_diff) + " " +
           ft_itos_width(pLocal->tm_hour, 2) + ":" + ft_itos_width(pLocal->tm_min, 2) + ":" + ft_itos_width(pLocal->tm_sec, 2) + " GMT";
  return (result);
}

// ! WARN: need to handle [Date + diff --> next day / previous day] ...etc
std::string HeaderType::getDateByHourOffset(int hour_diff)
{
  time_t curTime = time(NULL);          // get current time info
  struct tm* pLocal = gmtime(&curTime); // convert to struct for easy use
  if (pLocal == NULL)
  {
    // ...
    return ("null");
  }
  std::string result;
  result = GET_DAY(pLocal->tm_wday) + ", " + ft_itos_width(pLocal->tm_mday, 2) + " " + GET_MON(pLocal->tm_mon) + " " + ft_itos(pLocal->tm_year + 1900) + " " +
           ft_itos_width(pLocal->tm_hour + hour_diff, 2) + ":" + ft_itos_width(pLocal->tm_min, 2) + ":" + ft_itos_width(pLocal->tm_sec, 2) + " GMT";
  return (result);
}

HeaderType::t_pair HeaderType::CONTENT_LENGTH(const ssize_t& len)
{
  return (std::pair<std::string, std::string>("Content-Length", ft_itos(len)));
}

HeaderType::t_pair HeaderType::CONTENT_LANGUAGE(const std::string& language)
{
  return (std::pair<std::string, std::string>("Content-Language", language));
}

HeaderType::t_pair HeaderType::CONTENT_TYPE(const std::string& type)
{
  return (std::pair<std::string, std::string>("Content-type", type));
}

HeaderType::t_pair HeaderType::DATE()
{
  return (std::pair<std::string, std::string>("Date", getDate()));
}

HeaderType::t_pair HeaderType::SERVER(const std::string& serverName)
{
  return (std::pair<std::string, std::string>("Server", serverName));
}

HeaderType::t_pair HeaderType::CONNECTION(const std::string& connectionMode)
{
  return (std::pair<std::string, std::string>("Connection", connectionMode));
}

HeaderType::t_pair HeaderType::TRANSFER_ENCODING(const std::string& transfer_encoding_type)
{
  return (std::pair<std::string, std::string>("Transfer-Encoding", transfer_encoding_type));
}

HeaderType::t_pair HeaderType::ALLOW(const std::string& allowedMethod)
{
  return (std::pair<std::string, std::string>("Allow", allowedMethod));
}

HeaderType::t_pair HeaderType::LOCATION(const std::string& redirect_location)
{
  return (std::pair<std::string, std::string>("Location", redirect_location));
}

HeaderType::t_pair HeaderType::SET_COOKIE(const std::string& entity)
{
  return (std::pair<std::string, std::string>("Set-Cookie", entity));
}

/**----------------------
 * * HttpResponseHeader |
 *----------------------*/

HTTPResponseHeader::HTTPResponseHeader()
        :
        _version("HTTP/1.1"), _status_code(-1), _statusMessage("null")
{
  this->addHeader(HTTPResponseHeader::SERVER("null"));
  setDefaultHeaderDescription();
}

HTTPResponseHeader::HTTPResponseHeader(const std::string& version, const int& statusCode, const std::string& statusMessage, const std::string& serverName)
        :
        _version(version),
        _status_code(statusCode),
        _statusMessage(statusMessage)
{
  this->addHeader(HTTPResponseHeader::SERVER(serverName));
  setDefaultHeaderDescription();
}

HTTPResponseHeader& HTTPResponseHeader::operator=(const HTTPResponseHeader& header)
{
  _version = header._version;
  _status_code = header.getStatusCode();
  _statusMessage = header._statusMessage;
  _description = header._description;
  return (*this);
}

void HTTPResponseHeader::addHeader(const std::pair<std::string, std::string>& descriptionPair)
{
  this->_description[descriptionPair.first] = descriptionPair.second;
}

void HTTPResponseHeader::addHeader(const std::string& key, const std::string& value)
{
  this->_description[key] = value;
}

void HTTPResponseHeader::setVersion(const std::string& version)
{
  this->_version = version;
};

void HTTPResponseHeader::setStatus(const int& statusCode, const std::string& statusMessage)
{
  this->_status_code = statusCode;
  this->_statusMessage = statusMessage;
};

std::string HTTPResponseHeader::getVersion() const
{
  return this->_version;
};

int HTTPResponseHeader::getStatusCode() const
{
  return this->_status_code;
};

std::string HTTPResponseHeader::getStatusMessage() const
{
  return this->_statusMessage;
};

const std::map<std::string, std::string>& HTTPResponseHeader::getDescription() const
{
  return this->_description;
}

ssize_t HTTPResponseHeader::getContentLength() const
{
  std::string len = this->_description.find("Content-Length")->second;
  return ft_stoi(len);
}

std::string HTTPResponseHeader::toString() const
{
  // (1) if _status_code is out of range, throw error
  if (_status_code < 100 || _status_code > 599)
  {
    throw std::runtime_error("Status Code:" + ft_itos(_status_code) + " -> HttpResponse::toString() : status code is out of range\n");
  }

  // (2) if there is no status message, throw error
  if (_statusMessage == "null")
  {
    throw std::runtime_error("HttpResponse::toString() : status message unset\n");
  }

  // (3) if server name unset.
  if (this->_description.find("Server")->second == "null")
  {
    throw std::runtime_error("HttpResponse::toString() : server name unset.\n");
  }

  std::string header_message = _version + " " + ft_itos(_status_code) + " " + _statusMessage + "\r\n";
  HTTPResponseHeader::t_iterator itr = _description.find("Transfer-Encoding");
  itr = _description.begin();
  bool is_chunked = isTransferChunked();
  while (itr != _description.end())
  {
    if ((is_chunked && (itr->first == "Content-Length")) || (itr->first == "Content-Length" && itr->second == "-1"))
    {
      // ignore Content-Length
      itr++;
      continue;
    }
    header_message += (itr->first + ": " + itr->second + "\r\n");
    itr++;
  }
  return header_message;
}

bool HTTPResponseHeader::isTransferChunked() const
{
  bool is_chunked = false;
  HTTPResponseHeader::t_iterator itr = _description.find("Tranfer-Encoding");
  if (itr != _description.end() && itr->second.find("chunked") != std::string::npos)
  {
    is_chunked = true;
  }
  return (is_chunked);
}

void HTTPResponseHeader::setDefaultHeaderDescription()
{
  this->addHeader(HTTPResponseHeader::DATE());
  this->addHeader(HTTPResponseHeader::CONNECTION("keep-alive"));
  this->addHeader(HTTPResponseHeader::CONTENT_LENGTH(0));
}

HTTPResponseHeader::HTTPResponseHeader(const HTTPResponseHeader& header)
{
  *this = header;
}

/**----------------------
 * * HttpResponse       |
 *----------------------*/

HTTPResponse::HTTPResponse(const int& statusCode, const std::string& statusMessage, const std::string& serverName)
        :
        HTTPResponseHeader("HTTP/1.1", statusCode, statusMessage, serverName)
{
}

HTTPResponse::~HTTPResponse()
{
}

void HTTPResponse::setFd(const FileDescriptor& fd)
{
  _fileFd = fd;
}

HTTPResponseHeader HTTPResponse::getHeader() const
{
  return static_cast<HTTPResponseHeader>(*this);
}

FileDescriptor HTTPResponse::getFd() const
{
  return _fileFd;
}

void HTTPResponse::sendToClient(struct Context* context)
{
  context->res = this;
  // 인증된 세션의 경우 화면을 이동해도 로그인이 풀리지 않고 로그아웃하기 전까지 유지.
  if (this->getHeader().getStatusCode() >= 400)
    this->addHeader("Connection", "close");

  // * (0) Handle Cookie
  Server& server = context->manager->getMatchedServer(*context->req);
  server._sessionStorage.clearExpiredID(); // clear expired session.
  int sessionStatus = server.getSessionStatus(*context->req);
  if (sessionStatus == SESSION_UNSET) // create session_id and pass to client
  {
      std::string NEW_SESSION_ID = Session::gen_random_string(SESSION_ID_LENGH); // !WARN : this method is very insecure!
      this->addHeader(HTTPResponse::SET_COOKIE(std::string(SESSION_KEY) 
                                          + "=" + NEW_SESSION_ID + "; " 
                                          + "Expires=" + HTTPResponse::getDateByHourOffset(+SESSION_EXPIRE_HOUR)));
      server._sessionStorage.add(NEW_SESSION_ID, WS::Time().getByHourOffset(+SESSION_EXPIRE_HOUR)); 
  } 
  else if (sessionStatus == SESSION_INVALID) // unset client's session-cookie.
  {
    this->addHeader(HTTPResponse::SET_COOKIE(std::string(SESSION_KEY) + "=" 
                                          + Session::gen_random_string(SESSION_ID_LENGH) + "; " 
                                          + "Expires=" + HTTPResponse::getDateByYearOffset(-1) + ";"
                                          )); // set past date to delete cookie.
  }
  else // valid client session
    std::cout << "# Client session validated\n";


  // * (1) Send Header
  struct Context* newSendContext = new struct Context(context->fd, context->addr, socketSendHandler, context->manager);
  newSendContext->connectContexts = context->connectContexts;
  newSendContext->connectContexts->push_back(newSendContext);
  newSendContext->res = this;
  newSendContext->pipeFD[0] = context->pipeFD[0];
  newSendContext->pipeFD[1] = context->pipeFD[1];

  // add header content
  std::string header = this->getHeader().toString() + "\n";
  newSendContext->ioBuffer = new char[header.size()];
  memmove(newSendContext->ioBuffer, header.c_str(), header.size());
  newSendContext->bufferSize = header.size();
  newSendContext->threadKQ = context->threadKQ;
  newSendContext->req = context->req;

  struct kevent event;
  EV_SET(&event, newSendContext->fd, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, newSendContext);
  context->manager->attachNewEvent(newSendContext, event);
  if (context->res->_status_code >= 400)
  {
    EV_SET(&event, newSendContext->fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    context->manager->attachNewEvent(newSendContext, event);
  }
  // * (2) Send Body
  if (this->getFd() >= 0 && this->getContentLength() > 0 && this->getStatusCode() != ST_NO_CONTENT)
  {
    struct Context* newReadContext = new struct Context(context->fd, context->addr, bodyFdReadHandler, context->manager);
    newReadContext->connectContexts = context->connectContexts;
    newReadContext->connectContexts->push_back(newReadContext);
    newReadContext->res = this;
    newReadContext->req = context->req;
    newReadContext->threadKQ = context->threadKQ;
    newReadContext->pipeFD[0] = context->pipeFD[0];
    newReadContext->pipeFD[1] = context->pipeFD[1];
    struct kevent _event;
    EV_SET(&_event, newReadContext->res->_fileFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, newReadContext);
    context->manager->attachNewEvent(newReadContext, _event);
  }
  printLog(methodToString(context->req->method)  + "\t\t" + getClientIP(&context->addr) + '\t' + ft_itos(context->res->_status_code) + '\n', ((int)context->res->_status_code < 400) ? PRINT_BLUE : PRINT_MAGENTA);
  context->req = NULL;
}

void HTTPResponse::socketSendHandler(struct Context* context)
{
  if (DEBUG_MODE)
  {
    printLog("sk send handler called\n", PRINT_CYAN);
  }
  if (context->ioBuffer == NULL)
  {
    std::cout << "NULL\n";
    return ;
  }
  // 이 콜백은 socekt send 가능한 시점에서 호출되기 때문에, 이대로만 사용하면 된다.
  ssize_t sendSize;
  if ((sendSize = send(context->fd, context->ioBuffer, context->bufferSize, MSG_DONTWAIT)) < 0)
  {
    if (DEBUG_MODE)
    {
      printLog(strerror(errno), PRINT_YELLOW);
      std::cout << '\n' << context->bufferSize << "\n";
      printLog("error: " + getClientIP(&context->addr) + " : send failed\n", PRINT_RED);
    }
    return;
  }
  // partial send handle
  if (sendSize < context->bufferSize)
  {
    memmove(context->ioBuffer, &context->ioBuffer[sendSize], context->bufferSize - sendSize);
    context->bufferSize = context->bufferSize - sendSize;
  }
  else
  {
    // enable read event
    if (context->res->_fileFd > 0 && context->res->getContentLength() > context->totalIOSize)
    {
      struct Context* newReadContext = new struct Context(context->fd, context->addr, bodyFdReadHandler, context->manager);
      newReadContext->res = context->res;
      newReadContext->threadKQ = context->threadKQ;
      newReadContext->totalIOSize = context->totalIOSize;
      newReadContext->connectContexts = context->connectContexts;
      newReadContext->pipeFD[0] = context->pipeFD[0];
      newReadContext->pipeFD[1] = context->pipeFD[1];
      newReadContext->connectContexts->push_back(newReadContext);
      struct kevent _event;
      EV_SET(&_event, newReadContext->res->_fileFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, newReadContext);
      context->manager->attachNewEvent(newReadContext, _event);
    }
    else
    {
      // if bad request, close connection
      if (context->res->_status_code >= 400)
      {
        shutdown(context->fd, SHUT_RDWR);
        struct kevent ev[1];
        EV_SET(ev, context->fd, EVFILT_READ, EV_ADD, 0, 0, context);
        context->manager->attachNewEvent(context, ev[0]);
      }
      // delete sk event
      struct kevent ev[1];
      EV_SET(ev, context->fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
      context->manager->attachNewEvent(context, ev[0]);
    }
    // delete used buffer
    delete[] (context->ioBuffer);
    context->ioBuffer = NULL;
  }
}

void HTTPResponse::bodyFdReadHandler(struct Context* context)
{
  if (DEBUG_MODE)
  {
    printLog("file read handler called\n", PRINT_CYAN);
  }
  char* buffer = new char[BUFFER_SIZE];

  ssize_t current_rd_size = read(context->res->_fileFd, buffer, BUFFER_SIZE);
  if (current_rd_size < 0)
  {
    if (DEBUG_MODE)
    {
      printLog("error: client: " + getClientIP(&context->addr) + " : read failed\n", PRINT_RED);
    }
    close(context->res->_fileFd);
    delete[] (buffer);
    buffer = NULL;
  }
  else // 데이터가 들어왔다면, 소켓에 버퍼에 있는 데이터를 전송하는 socket send event를 등록.
  {
    context->totalIOSize += current_rd_size; // 읽은 길이를 누적.
    // Content_length와 누적 읽은 길이가 같아지면 file_fd 닫고 file_fd에 -1대입.
    bool is_read_finished = false;
    if (context->totalIOSize >= context->res->getContentLength())
    {
      close(context->res->_fileFd); // fd_file을 닫고.
      if (context->pipeFD[1] > 0)
        close(context->pipeFD[1]);
      context->res->_fileFd = -1;   // socketSendHandler가 file_fd가 -1이면 소켓을 종료.
      is_read_finished = true; // 마지막에 context delete하기 위함.
    }
    // ResponseContext를 만들어서 넘긴다.
    struct kevent event;
    struct Context* newSendContext = new struct Context(context->fd, context->addr, socketSendHandler, context->manager);
    newSendContext->connectContexts = context->connectContexts;
    newSendContext->connectContexts->push_back(newSendContext);
    newSendContext->res = context->res;
    newSendContext->ioBuffer = buffer;
    newSendContext->threadKQ = context->threadKQ;
    newSendContext->bufferSize = current_rd_size;
    newSendContext->totalIOSize = context->totalIOSize;
    newSendContext->pipeFD[0] = context->pipeFD[0];
    newSendContext->pipeFD[1] = context->pipeFD[1];
    EV_SET(&event, context->fd, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, newSendContext);
    context->manager->attachNewEvent(newSendContext, event);
    // read handler가 중복 호출 되는 것을 방지함
    if (!is_read_finished)
    {
      struct kevent ev[1];
      EV_SET(ev, context->res->_fileFd, EVFILT_READ, EV_DISABLE, 0, 0, context);
      context->manager->attachNewEvent(context, ev[0]);
    }
  }
}

std::string HTTPResponse::getClientIP(const struct sockaddr_in* addr)
{
  char str[INET_ADDRSTRLEN];
  const struct sockaddr_in* pV4Addr = addr;
  struct in_addr ipAddr = pV4Addr->sin_addr;
  inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);
  return (str);
}
