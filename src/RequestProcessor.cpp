#include "RequestProcessor.hpp"
#include "HTTPResponse.hpp"
#include "HTTPRequest.hpp"
#include "ServerManager.hpp"
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
    {
      return (true);
    }
  }
  return (false);
}

// ASSUMPTION : request contain complete header...
StatusCode RequestProcessor::checkValidHeader(const HTTPRequest& req)
{
  Server& matchedServer = _serverManager.getMatchedServer(req);

  // find _location
  Location* loc = matchedServer.getMatchedLocation(req);
  // check _location
  if (loc == NULL) // _root case
  {
    if (req.url != "/")
    {
      return (ST_NOT_FOUND);
    }
    if (!isAllowedMethod(matchedServer._allowMethods, req.method))
    {
      return (ST_METHOD_NOT_ALLOWED);
    }

    std::string contentLengthString = req.headers.at("Content-Length");
    if (contentLengthString.empty())
    {
      return (ST_LENGTH_REQUIRED);
    }
    int contentLength = ft_stoi(contentLengthString);
    if (matchedServer._clientMaxBodySize < contentLength)
    {
      return (ST_PAYLOAD_TOO_LARGE);
    }
  }
  else
  {
    if (!isAllowedMethod(loc->allowMethods, req.method))
    {
      return (ST_METHOD_NOT_ALLOWED);
    }

    std::string contentLengthString = req.headers.at("Content-Length");
    if (contentLengthString.empty())
    {
      return (ST_LENGTH_REQUIRED);
    }
    int contentLength = ft_stoi(contentLengthString);
    if (loc->clientMaxBodySize < contentLength)
    {
      return (ST_PAYLOAD_TOO_LARGE);
    }
  }
  return (ST_OK);
}

void RequestProcessor::processRequest(struct Context* context)
{
  // check context http request
  if (context == NULL || context->req == NULL)
  {
    throw (std::runtime_error("NULL context"));
  }
  // delete current event;
  struct kevent currentEvent;
  EV_SET(&currentEvent, context->fd, EVFILT_READ, EV_DELETE | EV_CLEAR, 0, 0, NULL);
  context->manager->attachNewEvent(context, currentEvent);
  // check request status
  HTTPRequest& req = *context->req;
  if (req.status == ERROR)
  {
    HTTPResponse* response = new HTTPResponse(ST_BAD_REQUEST, "bad request", context);

    // call response processor
    context->req = NULL;
    delete (&req);
    return;
  }
  else if (req.status == HEADEROK)
  {
    StatusCode status = checkValidHeader(req);

    if (status != ST_OK)
    {
      HTTPResponse* response = new HTTPResponse(status, "", context);

      // call response processor
      context->req = NULL;
      delete (&req);
      return;
    }
    if (req.method == GET || req.method == HEAD) // not consider body
    {
      Server& server = _serverManager.getMatchedServer(req);

      server.processRequest(context);
      context->req = NULL;
      delete (&req);
      return;
    }
    else
    {
      // attach new event to read remain data
      struct kevent newEvent;

      EV_SET(&newEvent, context->fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, context);
      _serverManager.attachNewEvent(context, newEvent); // callback Request Parser
      return;
    }
  }
  else if (req.status == READING)
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
    return;
  }
}

RequestProcessor::RequestProcessor(ServerManager& svm) :
        _serverManager(svm)
{

}

