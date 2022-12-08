#include "Server.hpp"
#include "HTTPResponse.hpp"
#include "ServerManager.hpp"
#include "CGI.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

// if there is no cookie in request --> return -1
// else, if valid id --> return  1
// else, if not valid --> return 0
int Server::getSessionStatus(const HTTPRequest& req)
{
  std::map<std::string, std::string>::const_iterator headerString_itr = req.headers.find("Cookie");
  if (headerString_itr != req.headers.end()) // if header has Cookie.
  {
    const std::string cookies = headerString_itr->second;
    const size_t id_loc = cookies.find(SESSION_KEY);
    if (id_loc != std::string::npos) // if session id exists,
    {
      const size_t idStartLoc = id_loc + std::string(SESSION_KEY).size() + 1;
      const std::string receivedId = cookies.substr(idStartLoc, SESSION_ID_LENGH);
      if (!(this->_sessionStorage.isValid_ID(receivedId))) // if sessionID does not match.
        return (SESSION_INVALID);
      else
        return (SESSION_VALID);
    }
  }
  return (SESSION_UNSET);
}

std::string Server::getRealFilePath(const HTTPRequest& req)
{
  Location* loc = getMatchedLocation(req);
  std::string filePath;

  if (loc == NULL)
  {
    // check request file exists on root
    if (req.url.rfind('/') == 0) // root case
    {
      filePath = req.url;
      if (filePath.length() == 1)
      {
        if (req.method == GET)
          filePath = _root + "/" + _index;
      }
      else
      {
        filePath = _root + "/" + filePath;
      }
    }
    else // there are no matched location
    {
      return ("FAILED");
    }
  }
  else
  {
    filePath = (loc->convertURLToLocationPath(req.url));
  }
  return (filePath);
}

static bool isAllowedMethod(std::vector<MethodType>& allowMethods, MethodType method)
{
  for (
          std::vector<MethodType>::iterator it = allowMethods.begin();
          it != allowMethods.end();
          ++it
          )
  {
    if (*it == method)
    {
      return (true);
    }
  }
  return (false);
}

Server::Server()
{
}

Server::~Server()
{
  shutdown(_serverFD, SHUT_RDWR);
}

void Server::openServer()
{
  int opt = 1;

  if ((this->_serverFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    throw (std::runtime_error("Create Socket failed\n"));
  }
  if (setsockopt(this->_serverFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
  {
    throw (std::runtime_error("Socket set option failed\n"));
  }
  if (fcntl(this->_serverFD, F_SETFL, O_NONBLOCK) < 0)
  {
    throw (std::runtime_error("fcntl non-block failed\n"));
  }
  if (bind(this->_serverFD, reinterpret_cast<const sockaddr*>(&this->_socketAddr), sizeof(this->_socketAddr)) < 0)
  {
    throw (std::runtime_error("Bind Socket failed\n"));
  }
  if (listen(this->_serverFD, LISTEN_QUEUE_SIZE) < 0)
  {
    throw (std::runtime_error("Listen Socket failed\n"));
  }
}

FileDescriptor Server::getErrorPageFd(const StatusCode& stCode)
{
  // if error_page is set in config file, then return it's fd
  std::map<StatusCode, std::string>::const_iterator itr = this->_errorPage.find(stCode);
  if (itr == _errorPage.end()) // if no suitable errPage
    return (-1);
  else
    return (open(itr->second.c_str(), O_RDONLY)); // will return -1 or regular FD
}

#define READ  (0)
#define WRITE (1)

static FileDescriptor createIndexPage(struct Context* context, const std::string& filePath, Location& location)
{
  std::string content = "";
  struct Context* newContext = new struct Context;
  *newContext = *context;
  FileDescriptor* pipe_fd = context->pipeFD;

  if (pipe(pipe_fd) < 0)
  {
    throw(std::runtime_error("createIndexPage : pipe() returned -1"));
  }
  // write html to pipe_fd[WRITE]
  const std::string html_start = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>autoindex</title></head><body>\n";
  content += html_start;
  // 줄 바꿈은 <br>만 넣으면 된다.
  const std::string body_1 = "<h1> index of " + filePath + "</h1>" + "<hr/>\n";
  content += body_1;

  DIR *dir = opendir(filePath.c_str());
  if (dir == NULL)
  {
    content += "Unable to parse directory's index\n";
  }
  else
  {
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
      if (strcmp(entry->d_name, ".") == 0 /* || strcmp(entry->d_name, "..") == 0*/)
        continue;
      struct stat sb;
      std::string body_2 = "<h4>";
      std::string subdir = filePath + "/" + entry->d_name;
      stat(subdir.c_str(), &sb);
      if (strcmp(entry->d_name, "..") == 0)
      {
        body_2 += "<a href=\"" + location._location + "/" + "\">" + std::string(entry->d_name) + "</a><br>";
      }
      else if (S_ISDIR(sb.st_mode)) // if directory
      {
        body_2 += "<a href=\"" + location._location + "/" + entry->d_name + "\">" + std::string(entry->d_name) + "</a><br>";
      }
      else // if file
      {
        body_2 += "<a href=\"" + filePath + "/" + entry->d_name + "\">" + std::string(entry->d_name) + "</a><br>\n";
      }
      content += (body_2 + "</h4>");
    }
    closedir(dir);
  }
  const std::string html_end = "</body></html>";
  content += html_end;
  // attach write pipe event
  const size_t CONTENT_SIZE = content.size();
  struct kevent ev;
  newContext->req = context->req;
  newContext->res = context->res;
  newContext->fd = context->fd;
  newContext->handler = writePipeHandler;
  newContext->connectContexts->push_back(newContext);
  newContext->ioBuffer = new char[CONTENT_SIZE];
  newContext->totalIOSize = CONTENT_SIZE;
  newContext->pipeFD[0] = context->pipeFD[0];
  newContext->pipeFD[1] = context->pipeFD[1];
  context->req = NULL;
  memcpy(newContext->ioBuffer, content.c_str(), CONTENT_SIZE);
  EV_SET(&ev, pipe_fd[WRITE], EVFILT_WRITE, EV_ADD, 0, 0, newContext);
  context->manager->attachNewEvent(newContext, ev);
  // FIX: kqueue에 등록해야 한다. kqueue쪽에서 close하는거에 이벤트가 발생하기 때문이다.
  return (pipe_fd[READ]);
}



// TODO: Location과 directory는 분리해야 한다.
// 127.0.0.1:4242/directory 로 보냈을때 location 정보가 YoupiBanne와 합쳐짐.
HTTPResponse* Server::processGETRequest(struct Context* context)
{
  HTTPRequest& req = *context->req;

  // check matched location
  std::string filePath = getRealFilePath(req);

  if (filePath == "FAILED")
  {
    const StatusCode RETURN_STATUS = ST_NOT_FOUND;
    HTTPResponse* response = new HTTPResponse(RETURN_STATUS, std::string("not found"), context->manager->getServerName(context->addr.sin_port));
    response->setFd(getErrorPageFd(RETURN_STATUS));
    return (response);
  }
  else if (isCGIRequest(filePath, getMatchedLocation(req)))
  {
    CGIProcess(context);
    return (NULL);
  }
  if (access(filePath.c_str(), R_OK) == FAILED)
  {
    if (DEBUG_MODE)
      printLog(filePath + " NOT FOUND\n", PRINT_RED);
    const StatusCode RETURN_STATUS = ST_NOT_FOUND;
    HTTPResponse* response = new HTTPResponse(RETURN_STATUS, std::string("not found"), context->manager->getServerName(context->addr.sin_port));
    response->setFd(getErrorPageFd(RETURN_STATUS));
    return (response);
  }
  else
  {
    HTTPResponse* response = new HTTPResponse(ST_OK, std::string("OK"), context->manager->getServerName(context->addr.sin_port));
    Location* loc = getMatchedLocation(req);
    if (loc && loc->_autoindex == true) // if autoindex : on
    {
      context->res = response;
      response->setFd(createIndexPage(context, filePath, *loc));
      return (NULL);
    }
    else // if autoindex : off
    {
      response->setFd(open(filePath.c_str(), O_RDONLY));
    }
    return (response);
  }
}

// Process POST reqeust
// Test on bash : curl -X POST http://127.0.0.1:4242/repository/test -d "Hello, World"
// 참고 내용 : http://blog.storyg.co/rest-api-response-body-best-pratics
HTTPResponse* Server::processPOSTRequest(struct Context* context)
{//std::cerr <<"inpost" << std::endl;
  HTTPRequest& req = *context->req;

  // check matched location
  std::string filePath = getRealFilePath(req);

  if (filePath == "FAILED")
  {
    const StatusCode RETURN_STATUS = ST_NOT_FOUND;
    HTTPResponse* response = new HTTPResponse(RETURN_STATUS, std::string("not found"), context->manager->getServerName(context->addr.sin_port));
    response->setFd(getErrorPageFd(RETURN_STATUS));
    return (response);
  }
  else if (isCGIRequest(filePath, getMatchedLocation(req)))
  {
    CGIProcess(context);
    return (NULL);
  }
  else
  {
    HTTPResponse* response = NULL;
    FileDescriptor writeFileFD = open(filePath.c_str(), O_WRONLY | O_CREAT | O_APPEND | O_NONBLOCK, 0777);
    if (writeFileFD <= -1 || access(filePath.c_str(), R_OK | W_OK) == FAILED)
    {
      const StatusCode RETURN_STATUS = ST_NOT_FOUND;
      response = new HTTPResponse(RETURN_STATUS, std::string("File is not available"), context->manager->getServerName(context->addr.sin_port));
      response->setFd(getErrorPageFd(RETURN_STATUS));
      return (response);
    }
    response = new HTTPResponse(ST_ACCEPTED, std::string("ACCEPTED"), context->manager->getServerName(context->addr.sin_port));
    response->addHeader("Content-Location", filePath);
    response->setFd(-1);
    // prepare event context
    struct Context* newContext = new struct Context(writeFileFD, context->addr, writeFileHandle, context->manager);
    newContext->res = response;
    newContext->req = context->req;
    newContext->threadKQ = context->threadKQ;
    newContext->connectContexts = context->connectContexts;
    newContext->connectContexts->push_back(newContext);
    newContext->totalIOSize = 0;
    newContext->pipeFD[0] = context->pipeFD[0];
    newContext->pipeFD[1] = context->pipeFD[1];
    response->_writeFD = writeFileFD;
    // attach event
    struct kevent event;
    EV_SET(&event, writeFileFD, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, newContext);
    context->manager->attachNewEvent(newContext, event);
    return (response);
  }
}

// Test on bash : curl -X PUT http://127.0.0.1:4242/repository/test -d "Hello, World"
HTTPResponse* Server::processPUTRequest(struct Context* context)
{
  HTTPRequest& req = *context->req;

  // check matched location
  std::string filePath = getRealFilePath(req);

  if (filePath == "FAILED")
  {
    const StatusCode RETURN_STATUS = ST_NOT_FOUND;
    HTTPResponse* response = new HTTPResponse(RETURN_STATUS, std::string("not found"), context->manager->getServerName(context->addr.sin_port));
    response->setFd(getErrorPageFd(RETURN_STATUS));
    return (response);
  }
  else
  {
    HTTPResponse* response = NULL;

    FileDescriptor writeFileFD = open(filePath.c_str(), O_WRONLY |O_CREAT | O_TRUNC | O_NONBLOCK, 0777);
    if (writeFileFD <= -1)
    {
      const StatusCode RETURN_STATUS = ST_BAD_REQUEST;
      response = new HTTPResponse(RETURN_STATUS, std::string("File is not available"), context->manager->getServerName(context->addr.sin_port));
      response->setFd(getErrorPageFd(RETURN_STATUS));
      return (response);
    }
    response = new HTTPResponse(ST_ACCEPTED, std::string("Accepted"), context->manager->getServerName(context->addr.sin_port));
    response->addHeader("Content-Location", filePath);
    response->setFd(-1);
    // prepare event context
    struct Context* newContext = new struct Context(writeFileFD, context->addr, writeFileHandle, context->manager);
    newContext->res = response;
    newContext->req = context->req;
    newContext->threadKQ = context->threadKQ;
    newContext->totalIOSize = 0;
    newContext->connectContexts = context->connectContexts;
    newContext->connectContexts->push_back(newContext);
    newContext->pipeFD[0] = context->pipeFD[0];
    newContext->pipeFD[1] = context->pipeFD[1];
    response->_writeFD = writeFileFD;
    // attach event
    struct kevent event;
    EV_SET(&event, writeFileFD, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, newContext);
    context->manager->attachNewEvent(newContext, event);
    return (response);
  }
}

HTTPResponse* Server::processHEADRequest(const struct Context* context)
{
  HTTPRequest& req = *context->req;

  // check matched location
  std::string filePath = getRealFilePath(req);

  if (filePath == "FAILED")
  {
    HTTPResponse* response = new HTTPResponse(ST_NOT_FOUND, std::string("not found"), context->manager->getServerName(context->addr.sin_port));
    response->setFd(-1);
    return (response);
  }
  // check this file is CGI path

  // check is valid file
  if (access(filePath.c_str(), R_OK) == FAILED)
  {
    HTTPResponse* response = new HTTPResponse(ST_NOT_FOUND, std::string("not found"), context->manager->getServerName(context->addr.sin_port));
    response->setFd(-1);
    return (response);
  }
  else
  {
    HTTPResponse* response = new HTTPResponse(ST_OK, std::string("OK"), context->manager->getServerName(context->addr.sin_port));
    response->setFd(-1);
    return (response);
  }
}

HTTPResponse* Server::processDELETERequest(const struct Context* context)
{
  HTTPRequest& req = *context->req;

  // check matched location
  std::string filePath = getRealFilePath(req);

  if (filePath == "FAILED")
  {
    const StatusCode RETURN_STATUS = ST_NOT_FOUND;
    HTTPResponse* response = new HTTPResponse(RETURN_STATUS, std::string("not found"), context->manager->getServerName(context->addr.sin_port));
    response->setFd(getErrorPageFd(RETURN_STATUS));
    return (response);
  }
  // check is valid file
  if (access(filePath.c_str(), R_OK | W_OK) == FAILED)
  {
    const StatusCode RETURN_STATUS = ST_NOT_FOUND;
    HTTPResponse* response = new HTTPResponse(RETURN_STATUS, std::string("not found"), context->manager->getServerName(context->addr.sin_port));
    response->setFd(getErrorPageFd(RETURN_STATUS));
    return (response);
  }
  else
  {
    HTTPResponse* response = new HTTPResponse(ST_ACCEPTED, std::string("Delete file requested"), context->manager->getServerName(context->addr.sin_port));
    if (unlink(filePath.c_str()) == FAILED)
      response->setStatus(ST_INTERNAL_SERVER_ERROR, "Server Error");
    response->setFd(-1);
    return (response);
  }
}

void Server::processRequest(struct Context* context)
{
  const HTTPRequest& req = *context->req;
  const MethodType requestMethod = req.method;
  HTTPResponse* response;

  // undefined method 는 앞서서 처리함.
  switch (requestMethod)
  {
    case GET:
    {
      response = (processGETRequest(context));
      break ;
    }
    case POST:
    {
      response = (processPOSTRequest(context));
      break ;
    }
    case PUT:
    {
      response = (processPUTRequest(context));
      break ;
    }
    case HEAD:
    {
      response = (processHEADRequest(context));
      break ;
    }
    case DELETE:
    {
      response = (processDELETERequest(context));
      break ;
    }
    default:
    {
      throw (std::runtime_error("Undefined method not handled\n")); // 발생하면 안되는 문제라서 의도적으로 핸들링 안함.
    }
  }
  if (!response)
  {
    return;
  }
  context->res = response;
  if (context->res && response->getFd() > 0)
    response->addHeader(HTTPResponseHeader::CONTENT_LENGTH(FdGetFileSize(response->getFd())));
  response->sendToClient(context);
}

const Location* getClosestMatchedLocation_recur(const Server& matchedServer, const std::string& subUrl)
{
  if (subUrl.empty()) // No location.
  {
    return (NULL);
  }
  for (
          std::vector<Location>::const_iterator it = matchedServer._locations.begin();
          it != matchedServer._locations.end();
          ++it
          ) {
    const Location &loc = *it;
    if (loc._location == subUrl) {
      return (&loc); // 경로를 뒤에서 하나씩 제거해보면서 location과 지속 비교.
    }
  }
  return (getClosestMatchedLocation_recur(matchedServer, subUrl.substr(0, subUrl.rfind('/'))));
}


Location* Server::getMatchedLocation(const HTTPRequest& req)
{
  size_t extensionPOS = req.url.find(".");
  size_t delimPOS;
  std::string extension;

  if (req.method == POST && extensionPOS != std::string::npos)
  {
    extension.assign("/cgi-");
    delimPOS = req.url.find("/", extensionPOS);
    if (delimPOS == std::string::npos)
    {
      extension.append(req.url.begin() + extensionPOS + 1, req.url.end());
    }
    else
    {
      extension.append(req.url.begin() + extensionPOS + 1, req.url.begin() + delimPOS);
    }
    for (std::vector<Location>::iterator it = _locations.begin(); it != _locations.end(); ++it)
    {
      if (it->_location == extension)
      {
        Location &loc = *it;
        return (&loc);
      }
    }
  }
  // location matching algorithm.
  return (const_cast<Location *>(getClosestMatchedLocation_recur(*this, req.url)));
}

// 만약 redirection이 맞다면, 두번째 인자*buf에 데이터를 넣어줌 + true 반환.
bool Server::isRedirect(const std::string& url, std::pair<StatusCode, std::string>* redir_buf) const
{
  if (this->_redirect.first >= 300 && this->_redirect.first <= 399 && !(_redirect.second.empty()))
  {
    redir_buf->first = this->_redirect.first;
    redir_buf->second = this->_redirect.second;
    return (true);
  }
  else
  {
    // if given argument matches Location + location has redirect.
    std::vector<Location>::const_iterator itr = _locations.begin();
    while (itr != this->_locations.end())
    {
      if (itr->isMatchedLocation(url) && itr->isRedirect())
      {
        redir_buf->first = itr->_redirect.first;
        redir_buf->second = itr->_redirect.second;
        return (true);
      }
      itr++;
    }
  }
  return (false);
}
