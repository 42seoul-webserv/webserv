#include "RequestProcessor.hpp"
#include "HTTPResponse.hpp"
#include "HTTPRequest.hpp"
#include "ServerManager.hpp"
#include "WebservDefines.hpp"

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
    // if not root
    if (req.url != "/")
    {
      return (ST_NOT_FOUND);
    }
    if (!isAllowedMethod(matchedServer._allowMethods, req.method))
    {
      return (ST_METHOD_NOT_ALLOWED);
    }

    try
    {
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
    catch (std::exception& e)
    {
      if (req.method != GET && req.method != HEAD)
      {
        return (ST_LENGTH_REQUIRED);
      }
    }
  }
  else
  {
    if (!isAllowedMethod(loc->allowMethods, req.method))
    {
      return (ST_METHOD_NOT_ALLOWED);
    }

    try
    {
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
    catch (std::exception& e)
    {
      if (req.method != GET && req.method != HEAD)
      {
        return (ST_LENGTH_REQUIRED);
      }
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
  if (req.status == ERROR)
  {
    if (DEBUG_MODE)
      printLog(*req.message, PRINT_RED);
    HTTPResponse* response = new HTTPResponse(ST_BAD_REQUEST, "bad request", context->manager->getServerName(context->addr.sin_port));
    context->res = response;

    response->addHeader(HTTPResponseHeader::CONTENT_LENGTH(0));
    response->sendToClient(context);
    return;
  }

  Server& server = _serverManager.getMatchedServer(req);
  if (req.status == HEADEROK || req.status == END)
  {
    StatusCode status = checkValidHeader(req);

    if (status != ST_OK)
    {
      HTTPResponse* response = new HTTPResponse(status, "", context->manager->getServerName(context->addr.sin_port));
      context->res = response;

      response->addHeader(HTTPResponseHeader::CONTENT_LENGTH(0));
      response->sendToClient(context);
      if (req.status == HEADEROK)
        delete (req.message);
      return;
    }
    // * if redirection.
    std::pair<StatusCode, std::string> redirect_data;
    if (server.isRedirect(req.url, &redirect_data))
    {
      HTTPResponse* response = new HTTPResponse(redirect_data.first, "redirect", context->manager->getServerName(context->addr.sin_port));
      context->res = response;
      // set location header.
      response->addHeader(HTTPResponse::LOCATION(redirect_data.second));
      response->addHeader(HTTPResponseHeader::CONTENT_LENGTH(0));
      response->sendToClient(context);
      return ;
    }
    if (req.method == GET || req.method == HEAD) // not consider body
    {
      server.processRequest(context);
      return;
    }
    else if (req.status == HEADEROK) // need to read body
    {
        return;
     }
    else
    {
      server.processRequest(context);
      return;
    }
  }
  else if (req.status == READING) // ignore request
  {
    return ;
  }
  else
  {

  }
}

RequestProcessor::RequestProcessor(ServerManager& svm) :
        _serverManager(svm)
{

}

