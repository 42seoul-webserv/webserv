#include "RequestProcessor.hpp"
#include "HttpResponse.hpp"
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
      return (true);
  }
  return (false);
}

// ASSUMPTION : request contain complete header...
StatusCode RequestProcessor::isValidHeader(const HTTPRequest &req)
{
  Server& matchedServer = _serverManager.getMatchedServer(req);

  // find location
  Location* loc = matchedServer.getMatchedLocation(req);
  // check location
  if (loc == NULL) // root case
  {
    if (req.url != "/")
      return (ST_NOT_FOUND);
    if (!isAllowedMethod(matchedServer._allowMethods, req.method))
      return (ST_METHOD_NOT_ALLOWED);

    std::string contentLengthString = req.header.at("Content-Length");
    if (contentLengthString.empty())
      return (ST_LENGTH_REQUIRED);
    int contentLength = ft_stoi(contentLengthString);
    if (matchedServer._clientMaxBodySize < contentLength)
      return (ST_PAYLOAD_TOO_LARGE);
  }
  else
  {
    if (!isAllowedMethod(loc->allowMethods, req.method))
      return (ST_METHOD_NOT_ALLOWED);

    std::string contentLengthString = req.header.at("Content-Length");
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
  // ! GET, HEAD 일 때, Header 가 완성 되면 이후 body 는 무시함.
  // file 이 실제로 존재 하는 지는 response 를 처리할 때 판단 (HEADER만 필요한 GET, HEADER와 이외의 경우가 다르기 때문)
  // check context http request

  // if context http request status == HEADER -> check is valid
  //  if header is invalid, response error status
  //  else if header is valid and GET, HEAD method, processRequest
  //  otherwise, read All data
  // else if status == END
  //  processResponse
  // else (status == ERROR)
  //  response error (server failed)
}

RequestProcessor::RequestProcessor(ServerManager &svm):
  _serverManager(svm)
{

}
