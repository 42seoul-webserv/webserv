#include "RequestProcessor.hpp"
#include "HttpResponse.hpp"

// ! --------------------------------------------------------
// ! FIXME: 이 함수는 다른 파일에 똑같이 존재함. 나중에 리팩토링할 때 include해서 사용할 것.
static std::string getClientIP(struct sockaddr_in* addr)
{
  char str[INET_ADDRSTRLEN];
  struct sockaddr_in* pV4Addr = addr;
  struct in_addr ipAddr = pV4Addr->sin_addr;
  inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);
  return (str);
}
// ! --------------------------------------------------------



void RequestProcessor::processGETMethod(const HTTPRequest &req,
                                                struct Context *context)
{
  /* 
  * ! TODO: 
  *   (1). location path searching alogirithm.
  *   (2). 처리 상황에 따라 적절한 response code 설정.
  ?   (3).  큰 이미지파일일 경우 chunked로 처리 (?)
  *   (4). 생성자에서 default response 설정.
  ?   (5). 큰 파일에 대한 read/write도 kevent를 통해 처리 (?)
  */ 


  // (1) init request object
  HttpResponse response(200, "OK", context);

  // (2) set requested file to body (이 read부분 또한 kevent를 통해 handling?)
  response.addHeader(HttpResponse::CONTENT_LANGUAGE("en-US"));
  response.addHeader(HttpResponse::CONTENT_TYPE("text/html"));
  response.setBodyandUpdateContentLength("../index.html");
  std::string res = response.toString();

  // (3) send string-converted Response Data to client.
  if (send(context->fd, res.data(), res.size(), 0) < 0)
  {
    printLog("error: client: " + getClientIP(&(context->addr)) + " : send failed\n", PRINT_RED);
    throw(std::runtime_error("Send Failed\n"));
  }
  printLog(getClientIP(&context->addr) + " send response\n", PRINT_BLUE);
  close(context->fd);
}

void RequestProcessor::processPOSTMethod(const HTTPRequest &req,
                                                 struct Context *context)
{

}

void RequestProcessor::processPUTMethod(const HTTPRequest &req,
                                                struct Context *context)
{

}

void RequestProcessor::processDELETEMethod(const HTTPRequest &req,
                                                   struct Context *context)
{

}

void RequestProcessor::processHEADMethod(const HTTPRequest &req,
                                                 struct Context *context)
{

}

void RequestProcessor::processPATCHMethod(const HTTPRequest &req,
                                                  struct Context *context)
{

}

void RequestProcessor::processCGI(const HTTPRequest &req,
                                          struct Context *context)
{
  // check request cgi is valid
  // if invalid -> response 404(?)

  pid_t pid;
  int pipe_stdin[2];
  int pipe_stdout[2];

  if (pipe(pipe_stdin) < 0)
    throw (std::runtime_error("Error : pipe failed\n"));
  if (pipe(pipe_stdout) < 0)
    throw (std::runtime_error("Error : pipe failed\n"));
  if ((pid = fork()) < 0)
    throw (std::runtime_error("Error : fork failed\n"));
  if (pid == 0)
  {
    // stdin -> pipe
    dup2(pipe_stdin[0], 0);
    close(pipe_stdin[1]);
    close(pipe_stdin[0]);
    // stdout -> pipe
    dup2(pipe_stdout[1], 1);
    close(pipe_stdout[1]);
    close(pipe_stdout[0]);
    // make argv
    // make envp
    // execve(cgiPath, argv, envp); -> envp has content-length
  }
  else
  {
    close(pipe_stdin[0]);
    close(pipe_stdout[1]);
    // event attach -> write body content to pipe -> partial write 에 대한 고려
    // event attach -> read cgi result from pipe -> partial read에 대한 고려와 result를 모두 얻었을 때 response 처리
  }
}

RequestProcessor::~RequestProcessor()
{

}
