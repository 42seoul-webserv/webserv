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
        filePath += _index;
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
    response->setFd(open(filePath.c_str(), O_RDONLY));
    return (response);
  }
}

HTTPResponse* Server::processPOSTRequest(const struct Context* context)
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
  if (access(filePath.c_str(), R_OK | W_OK) == FAILED)
  {
    HTTPResponse* response = new HTTPResponse(ST_NOT_FOUND, std::string("not found"), context->manager->getServerName(context->addr.sin_port));
    response->setFd(-1);
    return (response);
  }
  else
  {
    HTTPResponse* response = new HTTPResponse(ST_OK, std::string("OK"), context->manager->getServerName(context->addr.sin_port));
    response->setFd(-1);
    // FIXME: post file to server... -> add write event
    return (response);
  }
}

HTTPResponse* Server::processPUTRequest(const struct Context* context)
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
  if (access(filePath.c_str(), R_OK | W_OK) == FAILED)
  {
    HTTPResponse* response = new HTTPResponse(ST_NOT_FOUND, std::string("not found"), context->manager->getServerName(context->addr.sin_port));
    response->setFd(-1);
    return (response);
  }
  else
  {
    HTTPResponse* response = new HTTPResponse(ST_OK, std::string("Upload file requested"), context->manager->getServerName(context->addr.sin_port));
    response->setFd(-1);
    // add file to server...
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

HTTPResponse* Server::processPATCHRequest(const struct Context* context)
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
  if (access(filePath.c_str(), R_OK | W_OK) == FAILED)
  {
    HTTPResponse* response = new HTTPResponse(ST_NOT_FOUND, std::string("not found"), context->manager->getServerName(context->addr.sin_port));
    response->setFd(-1);
    return (response);
  }
  else
  {
    HTTPResponse* response = new HTTPResponse(ST_OK, std::string("Modify file requested"), context->manager->getServerName(context->addr.sin_port));
    response->setFd(-1);
    return (response);
  }
}

void Server::processRequest(const struct Context* context)
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
    case PATCH:
    {
      response = (processPATCHRequest(context));
      break ;
    }
    case DELETE:
    {
      response = (processDELETERequest(context));
      break ;
    }
    case UNDEFINED:
    {
      delete (context);
      throw (std::runtime_error("Undefined method not handled\n")); // 발생하면 안되는 문제라서 의도적으로 핸들링 안함.
    }
  }
  response->sendToClient(context->fd, context->addr, context->manager);
  delete (context);
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
