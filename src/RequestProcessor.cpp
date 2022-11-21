#include "RequestProcessor.hpp"

StatusCode RequestProcessor::isValidHeader(const HTTPRequest &req)
{
  std::vector<Server>& serverList = _serverManager.getServerList();
  for (
          std::vector<Server>::iterator it = serverList.begin();
          it != serverList.end();
          ++it
  )
  {
    Server& server = *it;

    if (server._serverName)

  }
  // check is valid method
  return (ST_OK);
}

HTTPResponse *RequestProcessor::createResponse(const HTTPRequest &req)
{
  return NULL;
}

void RequestProcessor::processRequest(struct Context *context)
{

}
