#include "RequestProcessor.hpp"

HTTPResponse RequestProcessor::processGETMethod(const HTTPRequest &req,
                                                struct Context *context)
{

  return (HTTPResponse());
}

HTTPResponse RequestProcessor::processPOSTMethod(const HTTPRequest &req,
                                                 struct Context *context)
{
  return (HTTPResponse());
}

HTTPResponse RequestProcessor::processPUTMethod(const HTTPRequest &req,
                                                struct Context *context)
{
  return (HTTPResponse());
}

HTTPResponse RequestProcessor::processDELETEMethod(const HTTPRequest &req,
                                                   struct Context *context)
{
  return (HTTPResponse());
}

HTTPResponse RequestProcessor::processHEADMethod(const HTTPRequest &req,
                                                 struct Context *context)
{
  return (HTTPResponse());
}

HTTPResponse RequestProcessor::processPATCHMethod(const HTTPRequest &req,
                                                  struct Context *context)
{
  return (HTTPResponse());
}

HTTPResponse RequestProcessor::processCGI(const HTTPRequest &req,
                                          struct Context *context)
{
  // request의 CGI 판별
  // CGI 프로세스를 fork 하고 environ으로 적절한 환경 변수를 건내준다.
  //execve(cgiPath, argv, envp); -> envp has content-length
  //body data send to pipe (stdin 으로 활용하게 될 것...)
  return (HTTPResponse());
}

RequestProcessor::~RequestProcessor()
{

}
