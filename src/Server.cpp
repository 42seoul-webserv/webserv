#include "Server.hpp"
#include <algorithm>
#include "HttpResponse.hpp"

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

HttpResponse &Server::processGETRequest(const HTTPRequest &req)
{
  // check valid path

  // check cgi first..
  // response body ( open file and add fd to res )
  /*
 * ! TODO:
 *   (1). location path searching alogirithm.
 *   (2). Request Data와 처리 상황에 따라 적절한 response code 설정.
 ?   (3).  큰 이미지파일일 경우 chunked로 처리 (?)
 // *   (4). 생성자에서 default response 설정.
 ?   (5). 큰 파일에 대한 read/write도 kevent를 통해 처리 (?)
 */


  // (1) init request object
  HttpResponse* response = new HttpResponse(200, "OK", context);

  // (2) set requested file to body (이 read부분 또한 kevent를 통해 handling?)
  response.addHeader(HttpResponse::CONTENT_LANGUAGE("en-US"));
  response.addHeader(HttpResponse::CONTENT_TYPE("text/html"));
  response.setBodyandUpdateContentLength("../index.html");
  std::string res = response.toString();
  return (*res);
}

HttpResponse &Server::processPOSTRequest(const HTTPRequest &req)
{
  HttpResponse* res = new HttpResponse();

  // check valid path
  // check cgi first
  // if upload file,
  return (*res);
}

HttpResponse &Server::processPUTRequest(const HTTPRequest &req)
{
  HttpResponse* res = new HttpResponse();

  return (*res);
}

HttpResponse &Server::processHEADRequest(const HTTPRequest &req)
{
  HttpResponse* res = new HttpResponse();

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

HttpResponse &Server::processRequest(const HTTPRequest &req)
{
  const MethodType requestMethod = req.getMethod();

  if (!isAllowedMethod(_allowMethods, requestMethod))
  {
    HttpResponse*const  res = new HttpResponse();
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
  HttpResponse* res = new HttpResponse();
  // error response
  return (*res);
}
