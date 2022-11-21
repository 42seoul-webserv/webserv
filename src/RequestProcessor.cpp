#include "RequestProcessor.hpp"
#include "HttpResponse.hpp"
#include "ServerManager.hpp"
#include <unistd.h>

StatusCode RequestProcessor::isValidHeader(const HTTPRequest &req)
{
  std::vector<Server> &serverList = _serverManager.getServerList();
  for (
          std::vector<Server>::iterator it = serverList.begin();
          it != serverList.end();
          ++it
          )
  {

  }

  return (ST_OK);
}

HttpResponse *RequestProcessor::createResponse(const HTTPRequest &req)
{
  return NULL;
}

void RequestProcessor::processRequest(struct Context *context)
{

}
