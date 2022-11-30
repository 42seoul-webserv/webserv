#include "Server.hpp"
#include "HTTPResponse.hpp"
#include "ServerManager.hpp"
#include <fcntl.h>
#include <unistd.h>

// TODO : autoindex

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
        filePath = _root + _index;
      }
      else
      {
        filePath = _root + filePath;
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

HTTPResponse* Server::processGETRequest(const struct Context* context)
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
  //if (CGI)
  //{
  //  cgiRequest(req, filePath);
  //  return ;
  //}
  // check is valid file
  if (access(filePath.c_str(), R_OK) == FAILED)
  {
    if (DEBUG_MODE)
      printLog(filePath + "NOT FOUND\n", PRINT_RED);
    HTTPResponse* response = new HTTPResponse(ST_NOT_FOUND, std::string("not found"), context->manager->getServerName(context->addr.sin_port));
    response->setFd(-1);
    return (response);
  }
  else
  {
    HTTPResponse* response = new HTTPResponse(ST_OK, std::string("OK"), context->manager->getServerName(context->addr.sin_port));
    response->setFd(open(filePath.c_str(), O_RDONLY));
    response->addHeader(HTTPResponse::CONTENT_LENGTH(FdGetFileSize(response->getFd())));

    return (response);
  }
}

// Process POST reqeust
// Test on bash : curl -X POST http://127.0.0.1:4242/repository/test -d "Hello, World"
// TODO: 현재는 POST 요청시 생성할 파일명까지 명시하지만,
// TODO: 사실 요청은 경로만 입력되있고 이걸 서버가 알아서 판단, 파일을 생성한뒤 그 파일에 대한 identifier를 response해야 함.
// TODO: 이 부분은 form-data 처리랑도 연관 있으니 추후 토의후 마저 구현할 것.
// 참고 내용 : http://blog.storyg.co/rest-api-response-body-best-pratics
HTTPResponse* Server::processPOSTRequest(struct Context* context)
{
  HTTPRequest& req = *context->req;

  // check matched location
  std::string filePath = getRealFilePath(req);
  std::cout << "POST filePath : " << filePath << std::endl;

  if (filePath == "FAILED")
  {
    HTTPResponse* response = new HTTPResponse(ST_NOT_FOUND, std::string("not found"), context->manager->getServerName(context->addr.sin_port));
    response->setFd(-1);
    return (response);
  }
  else
  {
    HTTPResponse* response = NULL;
    if (access(filePath.c_str(), F_OK) == FAILED) // if file doesn't exist
    {
      response = new HTTPResponse(ST_CREATED, std::string("Created"), context->manager->getServerName(context->addr.sin_port));
    }
    else // if file exist
    {
      response = new HTTPResponse(ST_OK, std::string("OK"), context->manager->getServerName(context->addr.sin_port));
    }
    response->setFd(-1);
    // attach write event
    FileDescriptor writeFileFD = open(filePath.c_str(), O_WRONLY | O_CREAT | O_APPEND | O_NONBLOCK);
    if (writeFileFD <= -1)
    {
      throw std::runtime_error("file open error\n");
    }
    struct Context* newContext = new struct Context(writeFileFD, context->addr, writeFileHandle, context->manager);
    newContext->res = response;
    newContext->req = new HTTPRequest(*context->req);
    newContext->fd = writeFileFD;
    newContext->threadKQ = context->threadKQ;
    newContext->connectContexts = context->connectContexts;
    newContext->connectContexts->push_back(newContext);
    // FIXME: 변수명 고칠 것! read_size가 아니라 write_size임
    newContext->total_read_size = 0; // 변수명을 고치지 않고 일단 이 변수 사용함..
    struct kevent event;
    EV_SET(&event, writeFileFD, EVFILT_WRITE, EV_ADD, 0, 0, newContext);
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
  std::cout << "PUT filePath : " << filePath << std::endl;

  if (filePath == "FAILED")
  {
    HTTPResponse* response = new HTTPResponse(ST_NOT_FOUND, std::string("not found"), context->manager->getServerName(context->addr.sin_port));
    response->setFd(-1);
    return (response);
  }
  else
  {
   HTTPResponse* response = NULL;
    if (access(filePath.c_str(), F_OK) == FAILED) // if file doesn't exist
    {
      response = new HTTPResponse(ST_CREATED, std::string("Created"), context->manager->getServerName(context->addr.sin_port));
    }
    else // if file exist
    {
      response = new HTTPResponse(ST_NO_CONTENT, std::string("OK"), context->manager->getServerName(context->addr.sin_port));
    }
    response->addHeader("Content-Location", filePath);
    response->setFd(-1);
    // attach write event
    FileDescriptor writeFileFD = open(filePath.c_str(), O_CREAT | O_TRUNC | O_WRONLY | O_NONBLOCK);
    if (writeFileFD <= -1)
    {
      throw std::runtime_error("file open error\n");
    }
    struct Context* newContext = new struct Context(writeFileFD, context->addr, writeFileHandle, context->manager);
    newContext->res = response;
    newContext->req = new HTTPRequest(*context->req);
    newContext->fd = writeFileFD;
    newContext->threadKQ = context->threadKQ;
    newContext->connectContexts = context->connectContexts;
    newContext->connectContexts->push_back(newContext);
    // FIXME: 변수명 고칠 것! read_size가 아니라 write_size임
    newContext->total_read_size = 0; // 변수명을 고치지 않고 일단 이 변수 사용함..
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
    HTTPResponse* response = new HTTPResponse(ST_NOT_FOUND, std::string("not found"), context->manager->getServerName(context->addr.sin_port));
    response->setFd(-1);
    return (response);
  }
  // check is valid file
  if (access(filePath.c_str(), R_OK | W_OK) == FAILED)
  {
    HTTPResponse* response = new HTTPResponse(ST_NOT_FOUND, std::string("not found"), context->manager->getServerName(context->addr.sin_port));
    response->setFd(-1);
    return (response);
  }
  else
  {
    HTTPResponse* response = new HTTPResponse(ST_OK, std::string("Delete file requested"), context->manager->getServerName(context->addr.sin_port));
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
  context->res = response;
  if (response->getFd() > 0)
    response->addHeader(HTTPResponseHeader::CONTENT_LENGTH(FdGetFileSize(response->getFd())));
  else
    response->addHeader(HTTPResponseHeader::CONTENT_LENGTH(0));
  response->sendToClient(context); // FIXME : 이런 형태로 고쳐져야함.
}

Location* Server::getMatchedLocation(const HTTPRequest& req)
{
  for (
          std::vector<Location>::iterator it = _locations.begin();
          it != _locations.end();
          ++it
          )
  {
    Location& loc = *it;

    if (loc.isMatchedLocation(req.url))
    {
      return (&loc);
    }
  }
  return (NULL); // _root case?
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
    std::vector<Location>::const_iterator itr;
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