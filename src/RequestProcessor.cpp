#include "RequestProcessor.hpp"
#include "HTTPResponse.hpp"
#include "HTTPRequest.hpp"
#include "ServerManager.hpp"
#include "WebservDefines.hpp"
#include "CGI.hpp"

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

// WARN: Test code!
StatusCode checkValidUrl_recur(const Server& matchedServer, const std::string& subUrl)
{
  if (subUrl.empty()) // if substr has no /
  {
    return (ST_NOT_FOUND);
  }
  for (
          std::vector<Location>::const_iterator it = matchedServer._locations.begin();
          it != matchedServer._locations.end();
          ++it
          ) {
    const Location &loc = *it;
    if (loc._location == subUrl) {
      return (ST_OK); // 경로를 뒤에서 하나씩 제거해보면서 location과 지속 비교.
    }
  }
  return (checkValidUrl_recur(matchedServer, subUrl.substr(0, subUrl.rfind('/'))));
}


StatusCode RequestProcessor::checkValidHeader(const HTTPRequest& req)
{
  Server& matchedServer = _serverManager.getMatchedServer(req);

  // find _location
  Location* loc = matchedServer.getMatchedLocation(req);
  // check _location
  if (loc == NULL) // _root case
  {
    if (!isAllowedMethod(matchedServer._allowMethods, req.method))
    {
      return (ST_METHOD_NOT_ALLOWED);
    }
    if (req.chunkedFlag)
    {
      return (ST_OK);
    }
    try
    {
      std::map<std::string, std::string>::const_iterator it = req.headers.find("Content-Length");
      if (it == req.headers.end())
        throw (std::logic_error(""));
      if (it->second.empty())
      {
        return (ST_LENGTH_REQUIRED);
      }
      int contentLength = ft_stoi(it->second);
      if (loc->clientMaxBodySize < contentLength)
      {
        return (ST_PAYLOAD_TOO_LARGE);
      }
    }
    catch (std::exception& e)
    {
      if (req.method != GET && req.method != HEAD && req.method != DELETE)
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
    if (isCGIRequest(loc))
    {
      return (ST_OK);
    }
    if (req.chunkedFlag == true && req.body->size() <= static_cast<size_t>(loc->clientMaxBodySize))
    {
      return (ST_OK);
    }
    if (static_cast<size_t>(loc->clientMaxBodySize) < req.body->size())
    {
      return (ST_PAYLOAD_TOO_LARGE);
    }
    try
    {
      std::map<std::string, std::string>::const_iterator it = req.headers.find("Content-Length");
      if (it == req.headers.end())
        throw (std::logic_error(""));
      if (it->second.empty())
      {
        return (ST_LENGTH_REQUIRED);
      }
      int contentLength = ft_stoi(it->second);
      if (loc->clientMaxBodySize < contentLength)
      {
        return (ST_PAYLOAD_TOO_LARGE);
      }
    }
    catch (std::exception& e)
    {
      if (req.method != GET && req.method != HEAD && req.method != DELETE)
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
    Server& server = _serverManager.getMatchedServer(req);

    context->res = response;
    response->addHeader(HTTPResponseHeader::CONTENT_LENGTH(0));
    response->setFd(server.getErrorPageFd(ST_BAD_REQUEST));
    if (response->getFd() > 0)
      response->addHeader(HTTPResponseHeader::CONTENT_LENGTH(FdGetFileSize(response->getFd())));
    response->sendToClient(context);
    return;
  }

  Server& server = _serverManager.getMatchedServer(req);
  if (req.status == HEADEROK || req.status == END)
  {
    StatusCode status = checkValidHeader(req);

    if (status != ST_OK)
    {
      HTTPResponse* response = new HTTPResponse(status, "No", context->manager->getServerName(context->addr.sin_port));
      context->res = response;

      response->addHeader(HTTPResponseHeader::CONTENT_LENGTH(0));
      response->setFd(server.getErrorPageFd(status));
      if (response->getFd() > 0)
        response->addHeader(HTTPResponseHeader::CONTENT_LENGTH(FdGetFileSize(response->getFd())));
      response->sendToClient(context);
      if (req.status == HEADEROK)
      {
        delete (req.message);
        req.message = NULL;
      }
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
      response->setFd(-1);
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

