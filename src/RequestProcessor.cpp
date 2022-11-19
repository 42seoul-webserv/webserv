#include "RequestProcessor.hpp"

void RequestProcessor::processGETMethod(const HTTPRequest &req,
                                                struct Context *context)
{


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
