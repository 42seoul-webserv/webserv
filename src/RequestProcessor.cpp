#include "RequestProcessor.hpp"
#include "HttpResponse.hpp"
#include "HTTPRequest.hpp"
#include "ServerManager.hpp"
#include <unistd.h>

static bool isAllowedMethod(std::vector<MethodType>& allowMethods, MethodType method)
{
<<<<<<< HEAD
  /* 
  * ! TODO: 
  *   (1). location path searching alogirithm.
  *   (2). Request Data와 처리 상황에 따라 적절한 response code 설정.
  ?   (3).  큰 이미지파일일 경우 chunked로 처리 (?)
  // *   (4). 생성자에서 default response 설정.
  ?   (5). 큰 파일에 대한 read/write도 kevent를 통해 처리 (?)
  */ 


  // (1) init request object
  HTTPResponse response(context);

  // (2) set requested file to body (이 read부분 또한 kevent를 통해 handling?)
  response.addHeader(HTTPResponse::CONTENT_LANGUAGE("en-US"));
  response.addHeader(HTTPResponse::CONTENT_TYPE("text/html"));
  // response.setBodyandUpdateContentLength("../index.html");
  // std::string res = response.toString();

  // (3) send string-converted Response Data to client.
  // if (send(context->fd, res.data(), res.size(), 0) < 0)
  // {
    // printLog("error: client: " + getClientIP(&(context->addr)) + " : send failed\n", PRINT_RED);
    // throw(std::runtime_error("Send Failed\n"));
  // }
  // printLog(getClientIP(&context->addr) + " send response\n", PRINT_BLUE);
  // close(context->fd);
}

void RequestProcessor::processPOSTMethod(const HttpRequest &req,
                                                 struct Context *context)
{

}

void RequestProcessor::processPUTMethod(const HttpRequest &req,
                                                struct Context *context)
{

}

void RequestProcessor::processDELETEMethod(const HttpRequest &req,
                                                   struct Context *context)
{

}

void RequestProcessor::processHEADMethod(const HttpRequest &req,
                                                 struct Context *context)
{

=======
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
>>>>>>> upstream/develop
}

// ASSUMPTION : request contain complete header...
StatusCode RequestProcessor::checkValidHeader(const HTTPRequest &req)
{
  Server& matchedServer = _serverManager.getMatchedServer(req);

  // find _location
  Location* loc = matchedServer.getMatchedLocation(req);
  // check _location
  if (loc == NULL) // _root case
  {
    if (req._url != "/")
      return (ST_NOT_FOUND);
    if (!isAllowedMethod(matchedServer._allowMethods, req._method))
      return (ST_METHOD_NOT_ALLOWED);

    std::string contentLengthString = req._headers.at("Content-Length");
    if (contentLengthString.empty())
      return (ST_LENGTH_REQUIRED);
    int contentLength = ft_stoi(contentLengthString);
    if (matchedServer._clientMaxBodySize < contentLength)
      return (ST_PAYLOAD_TOO_LARGE);
  }
  else
  {
    if (!isAllowedMethod(loc->allowMethods, req._method))
      return (ST_METHOD_NOT_ALLOWED);

    std::string contentLengthString = req._headers.at("Content-Length");
    if (contentLengthString.empty())
      return (ST_LENGTH_REQUIRED);
    int contentLength = ft_stoi(contentLengthString);
    if (loc->clientMaxBodySize < contentLength)
      return (ST_PAYLOAD_TOO_LARGE);
  }
  return (ST_OK);
}

void RequestProcessor::processRequest(struct Context *context)
{
  // check context http request
  if (context == NULL || context->req == NULL)
    throw (std::runtime_error("NULL context"));
  // delete current event;
  struct kevent currentEvent;
  EV_SET(&currentEvent, context->fd, EVFILT_READ, EV_DELETE | EV_CLEAR, 0, 0, NULL);
  context->manager->attachNewEvent(context, currentEvent);
  // check request status
  HTTPRequest& req = *context->req;
  if (req._status == ERROR)
  {
    HttpResponse* response = new HttpResponse(ST_BAD_REQUEST, "bad request", context);

    // call response processor
    context->req = NULL;
    delete (&req);
    return ;
  }
  else if (req._status == HEADEROK)
  {
    StatusCode status = checkValidHeader(req);

    if (status != ST_OK)
    {
      HttpResponse* response = new HttpResponse(status, "", context);

      // call response processor
      context->req = NULL;
      delete (&req);
      return ;
    }
    if (req._method == GET || req._method == HEAD) // not consider body
    {
      Server& server = _serverManager.getMatchedServer(req);

      server.processRequest(context);
      context->req = NULL;
      delete (&req);
      return ;
    }
    else
    {
      // attach new event to read remain data
      struct kevent newEvent;

      EV_SET(&newEvent, context->fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, context);
      _serverManager.attachNewEvent(context, newEvent); // callback Request Parser
      return ;
    }
  }
  else if (req._status == READING)
  {
    delete (&req);
    throw (std::runtime_error("READING Status is invalid in processing\n"));
  }
  else // status == END
  {
    ServerManager& svm = *context->manager;
    Server& server = svm.getMatchedServer(req);

    server.processRequest(context);
    delete (&req);
    return ;
  }
}

RequestProcessor::RequestProcessor(ServerManager &svm):
  _serverManager(svm)
{

}

