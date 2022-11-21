#include "Server.hpp"
#include <algorithm>
#include "HTTPResponse.hpp"

static const struct sockaddr_in DEFAULT_ADDR ={
        inet_addr("0.0.0.0"),
        AF_INET,
        htons(42424),
};

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

HTTPResponse &Server::processGETRequest(const HTTPRequest &req)
{
  HTTPResponse* res = new HTTPResponse();

  // check valid path

  // check cgi first..
  // response body ( open file and add fd to res )
  return (*res);
}

HTTPResponse &Server::processPOSTRequest(const HTTPRequest &req)
{
  HTTPResponse* res = new HTTPResponse();

  // check valid path
  // check cgi first
  // if upload file,
  return (*res);
}

HTTPResponse &Server::processPUTRequest(const HTTPRequest &req)
{
  HTTPResponse* res = new HTTPResponse();

  return (*res);
}

HTTPResponse &Server::processHEADRequest(const HTTPRequest &req)
{
  HTTPResponse* res = new HTTPResponse();

  return (*res);
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
      return (true);
  }
  return (false);
}

HTTPResponse &Server::processRequest(const HTTPRequest &req)
{
  const MethodType requestMethod = req.getMethod();

  if (!isAllowedMethod(_allowMethods, requestMethod))
  {
    HTTPResponse*const  res = new HTTPResponse();
    // 405 not allowed method response
    return (*res);
  }
  switch (requestMethod)
  {
    case GET:
    {
      return (processGETRequest(req));
    }
    case POST:
    {
      return (processPOSTRequest(req));
    }
    case PUT:
    {
      return (processPUTRequest(req));
    }
    case HEAD:
    {
      return (processHEADRequest(req));
    }
    case PATCH:
    {
      return (processPATCHRequest(req));
    }
    case DELETE:
    {
      return (processDELETERequest(req));
    }
  }
  HTTPResponse* res = new HTTPResponse();
  // error response
  return (*res);
}
