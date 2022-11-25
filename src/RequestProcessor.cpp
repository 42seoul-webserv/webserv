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
  // check request status
  HTTPRequest& req = *context->req;
  Server& server = _serverManager.getMatchedServer(req);

  if (req.status == ERROR)
  {
    context->req = NULL;
    delete (&req);
    // response err
    HTTPResponse* response = new HTTPResponse(ST_BAD_REQUEST, "bad request", context->manager->getServerName(context->addr.sin_port));
    response->sendToClient(context->fd, context->addr, &_serverManager);
    delete (context);
    return;
  }
  else if (req.status == HEADEROK)
  {
    StatusCode status = checkValidHeader(req);

    if (status != ST_OK)
    {
      context->req = NULL;
      delete (&req);
      // response err
      HTTPResponse* response = new HTTPResponse(status, "", context->manager->getServerName(context->addr.sin_port));
      response->sendToClient(context->fd, context->addr, &_serverManager);
      delete (context);
      return;
    }
    // * if redirection.
    std::pair<StatusCode, std::string> redirect_data;
    if (server.isRedirect(req.url, &redirect_data))
    {
      context->req = NULL;
      delete (&req);
      HTTPResponse* response = new HTTPResponse(redirect_data.first, "redirect", context->manager->getServerName(context->addr.sin_port));
      // set location header.
      response->addHeader(HTTPResponse::LOCATION(redirect_data.second));
      response->sendToClient(context->fd, context->addr, &_serverManager);
      delete (context);
      return ;
    }
    if (req.method == GET || req.method == HEAD) // not consider body
    {
      context->req = NULL;
      delete (&req);
      // call response processor
//      Server& server = _serverManager.getMatchedServer(req);
      server.processRequest(context);
      return;
    }
    else // need to read body
    {
      return;
    }
  }
  else if (req.status == READING) // ignore request
  {
    return ;
  }
  else // status == END
  {
    context->req = NULL;
    delete (&req);
    // call response processor
//    Server& server = _serverManager.getMatchedServer(req);
    server.processRequest(context);
    return;
  }
}

RequestProcessor::RequestProcessor(ServerManager& svm) :
        _serverManager(svm)
{

}

