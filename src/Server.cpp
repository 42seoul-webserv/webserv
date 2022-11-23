#include "Server.hpp"
#include "HttpResponse.hpp"
#include "ServerManager.hpp"
#include <fcntl.h>
#include <unistd.h>

static bool isAllowedMethod(std::vector<MethodType>& allowMethods, MethodType method)
{
  for (
          std::vector<MethodType>::iterator it = allowMethods.begin();
          it != allowMethods.end();
          ++it
  )
  {
    if (*it == method)
      return (true);
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
    throw (std::runtime_error("Create Socket failed\n"));
  if (setsockopt(this->_serverFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    throw (std::runtime_error("Socket set option failed\n"));
  if (fcntl(this->_serverFD, F_SETFL, O_NONBLOCK) < 0)
    throw (std::runtime_error("fcntl non-block failed\n"));
  if (bind(this->_serverFD, reinterpret_cast<const sockaddr *>(&this->_socketAddr), sizeof(this->_socketAddr)) < 0)
    throw (std::runtime_error("Bind Socket failed\n"));
  if (listen(this->_serverFD, LISTEN_QUEUE_SIZE) < 0)
    throw (std::runtime_error("Listen Socket failed\n"));
}

HttpResponse &Server::processGETRequest(const struct Context* context)
{
  HTTPRequest& req = *context->req;

  // check matched location
  Location* matchedLocation = getMatchedLocation(req);
  std::string filePath;
  if (matchedLocation == NULL)
  {
    // check request file exists on root
    if (req._url.rfind('/') == 0) // root case
    {
      filePath = req._url;
      if (filePath.length() == 1)
      {
        filePath += _index;
      }
    }
    else // there are no matched location
    {
      HttpResponse* response = new HttpResponse(ST_NOT_FOUND, std::string("not found"), context);
      return (*response);
    }
  }
  else
  {
    filePath = matchedLocation->convertURLToLocationPath(req._url);
  }
  // check this file is CGI path

  // check is valid file
  if (access(filePath.c_str(), R_OK) == FAILED)
  {
    HttpResponse* response = new HttpResponse(ST_NOT_FOUND, std::string("not found"), context);
    return (*response);
  }
  else
  {
    HttpResponse* response = new HttpResponse(ST_OK, std::string("not found"), context);
    response->setBody(filePath); // FIXME: fd로 변경 될 것임.
    return (*response);
  }
}

HttpResponse &Server::processPOSTRequest(const struct Context* context)
{
  HTTPRequest& req = *context->req;

  // check matched location
  Location* matchedLocation = getMatchedLocation(req);
  std::string filePath;
  if (matchedLocation == NULL)
  {
    // check request file exists on root
    if (req._url.rfind('/') == 0) // root case
    {
      filePath = req._url;
      if (filePath.length() == 1)
      {
        filePath += _index;
      }
    }
    else // there are no matched location
    {
      HttpResponse* response = new HttpResponse(ST_NOT_FOUND, std::string("not found"), context);
      return (*response);
    }
  }
  else
  {
    filePath = matchedLocation->convertURLToLocationPath(req._url);
  }
  // check this file is CGI path

  // check is valid file
  if (access(filePath.c_str(), R_OK | W_OK) == FAILED)
  {
    HttpResponse* response = new HttpResponse(ST_NOT_FOUND, std::string("not found"), context);
    return (*response);
  }
  else
  {
    HttpResponse* response = new HttpResponse(ST_OK, std::string("not found"), context);
    response->setBody(filePath); // FIXME : POST에 맞는 결과가 나올 것.
    return (*response);
  }
}

HttpResponse &Server::processPUTRequest(const struct Context* context)
{
  HTTPRequest& req = *context->req;

  // check matched location
  Location* matchedLocation = getMatchedLocation(req);
  std::string filePath;
  if (matchedLocation == NULL)
  {
    // check request file exists on root
    if (req._url.rfind('/') == 0) // root case
    {
      filePath = req._url;
      if (filePath.length() == 1)
      {
        filePath += _index;
      }
    }
    else // there are no matched location
    {
      HttpResponse* response = new HttpResponse(ST_NOT_FOUND, std::string("not found"), context);
      return (*response);
    }
  }
  else
  {
    filePath = matchedLocation->convertURLToLocationPath(req._url);
  }
  // check this file is CGI path

  // check is valid file
  if (access(filePath.c_str(), R_OK | W_OK) == FAILED)
  {
    HttpResponse* response = new HttpResponse(ST_NOT_FOUND, std::string("not found"), context);
    return (*response);
  }
  else
  {
    HttpResponse* response = new HttpResponse(ST_OK, std::string("not found"), context);
    response->setBody(filePath); // FIXME:  결과 없음 (-1)
    return (*response);
  }
}

HttpResponse &Server::processHEADRequest(const struct Context* context)
{
  HTTPRequest& req = *context->req;

  // check matched location
  Location* matchedLocation = getMatchedLocation(req);
  std::string filePath;
  if (matchedLocation == NULL)
  {
    // check request file exists on root
    if (req._url.rfind('/') == 0) // root case
    {
      filePath = req._url;
      if (filePath.length() == 1)
      {
        filePath += _index;
      }
    }
    else // there are no matched location
    {
      HttpResponse* response = new HttpResponse(ST_NOT_FOUND, std::string("not found"), context);
      return (*response);
    }
  }
  else
  {
    filePath = matchedLocation->convertURLToLocationPath(req._url);
  }
  // check this file is CGI path

  // check is valid file
  if (access(filePath.c_str(), R_OK) == FAILED)
  {
    HttpResponse* response = new HttpResponse(ST_NOT_FOUND, std::string("not found"), context);
    return (*response);
  }
  else
  {
    HttpResponse* response = new HttpResponse(ST_OK, std::string("not found"), context);
    response->setBody(filePath); // FIXME: HEADER만 필요해서 안쓸거임 (-1)
    return (*response);
  }
}


HttpResponse &Server::processDELETERequest(const struct Context* context)
{
  HTTPRequest& req = *context->req;

  // check matched location
  Location* matchedLocation = getMatchedLocation(req);
  std::string filePath;
  if (matchedLocation == NULL)
  {
    // check request file exists on root
    if (req._url.rfind('/') == 0) // root case
    {
      filePath = req._url;
      if (filePath.length() == 1)
      {
        filePath += _index;
      }
    }
    else // there are no matched location
    {
      HttpResponse* response = new HttpResponse(ST_NOT_FOUND, std::string("not found"), context);
      return (*response);
    }
  }
  else
  {
    filePath = matchedLocation->convertURLToLocationPath(req._url);
  }
  // check this file is CGI path

  // check is valid file
  if (access(filePath.c_str(), R_OK | W_OK) == FAILED)
  {
    HttpResponse* response = new HttpResponse(ST_NOT_FOUND, std::string("not found"), context);
    return (*response);
  }
  else
  {
    HttpResponse* response = new HttpResponse(ST_OK, std::string("not found"), context);
    response->setBody(filePath); // FIXME:  결과 없음 (-1)
    return (*response);
  }
}

HttpResponse &Server::processPATCHRequest(const struct Context* context)
{
  HTTPRequest& req = *context->req;

  // check matched location
  Location* matchedLocation = getMatchedLocation(req);
  std::string filePath;
  if (matchedLocation == NULL)
  {
    // check request file exists on root
    if (req._url.rfind('/') == 0) // root case
    {
      filePath = req._url;
      if (filePath.length() == 1)
      {
        filePath += _index;
      }
    }
    else // there are no matched location
    {
      HttpResponse* response = new HttpResponse(ST_NOT_FOUND, std::string("not found"), context);
      return (*response);
    }
  }
  else
  {
    filePath = matchedLocation->convertURLToLocationPath(req._url);
  }
  // check this file is CGI path

  // check is valid file
  if (access(filePath.c_str(), R_OK | W_OK) == FAILED)
  {
    HttpResponse* response = new HttpResponse(ST_NOT_FOUND, std::string("not found"), context);
    return (*response);
  }
  else
  {
    HttpResponse* response = new HttpResponse(ST_OK, std::string("not found"), context);
    response->setBody(filePath); // FIXME:  결과 없음 (-1)
    return (*response);
  }
}

void Server::processRequest(const struct Context* context)
{
  const HTTPRequest& req = *context->req;
  const MethodType requestMethod = req._method;

  // undefined method 는 앞서서 처리함.
  switch (requestMethod)
  {
    case GET:
    {
      // call response processor
      HttpResponse& res = (processGETRequest(context));

      return ;
    }
    case POST:
    {
      HttpResponse& res = (processPOSTRequest(context));
      return ;
    }
    case PUT:
    {
      HttpResponse& res = (processPUTRequest(context));
      return ;
    }
    case HEAD:
    {
      HttpResponse& res = (processHEADRequest(context));
      return ;
    }
    case PATCH:
    {
      HttpResponse& res = (processPATCHRequest(context));
      return ;
    }
    case DELETE:
    {
      HttpResponse& res = (processDELETERequest(context));
      return ;
    }
    case UNDEFINED:
      throw (std::runtime_error("Undefined method not handled\n")); // 발생하면 안되는 문제라서 의도적으로 핸들링 안함.
  }
}

Location *Server::getMatchedLocation(const HTTPRequest& req)
{
  for (
          std::vector<Location>::iterator it = _locations.begin();
          it != _locations.end();
          ++it
  )
  {
    Location& loc = *it;

    if (loc.isMatchedLocation(req._url))
      return (&loc);
  }
  return (NULL); // _root case?
}
