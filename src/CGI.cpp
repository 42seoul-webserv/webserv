#include "CGI.hpp"
# define P_W	1
# define P_R	0

CGI::CGI()
{
  envCount = 0;
  path = new char[PATH_MAX];
  path[PATH_MAX - 1] = '\0';
  cmd = new char*[3];
  cmd[0] = new char[PATH_MAX];
  cmd[1] = new char[PATH_MAX];
  cmd[2] = NULL;
  env = new char*[ENVCOUNT];
}

CGI::~CGI()
{
  delete []path;
  delete []cmd[0];
  delete []cmd[1];
  delete []cmd;
  for (size_t i = 0; env[i] != NULL && i < envCount; ++i)
  {
    delete []env[i];
  }
  delete []env;
}

void CGI::parseBody(HTTPResponse* res, std::string message)
{
  int fd[2];
  res->addHeader("Content-Length", ft_itos(message.size()));
  if (pipe(fd) < 0)
  {
    throw (std::runtime_error("cgi pipe boom"));
  }
  write(fd[P_W], message.c_str(), message.size());
  close(fd[P_W]);
  res->setFd(fd[P_R]);
}

void CGI::parseHeader(HTTPResponse* res, std::string &message)
{
  std::string key;
  std::string buffer;
  size_t endPOS;
  std::string::iterator it = message.begin();

  endPOS = message.find("\r\n\r\n") + 2;
  if (endPOS == std::string::npos)
  {
    throw(std::logic_error("cgi header syntax error"));
  }
  for ( size_t i = 0; i != endPOS; ++i )
  {
    if (it[i] == '\r' && it[i + 1] == '\n')
    {
      if (!key.empty() && !buffer.empty())
      {
        res->addHeader(key, buffer);
        buffer.clear();
        key.clear();
        i++;
        continue;
      }
      else
      {
        throw (std::logic_error("header key, value error\n"));
      }
    }
    if (it[i] == ':' && key.empty())
    {
      key = buffer;
      buffer.clear();
      if (it[i + 1] == ' ' || it[i + 1] == '\t')
      {
        i++;
      }
    }
    else
    {
      buffer += it[i];
    }
  }
  message.erase(0, endPOS + 2);
}

void CGI::parseStartLine(struct Context* context, std::string &message)
{
  std::string::iterator it;
  std::string statusmessage;
  size_t statuscode;
  std::string buffer;
  size_t end = 0;

  end = message.find("\r\n");
  if (end == std::string::npos)
  {
    throw (std::logic_error("startline ERROR"));
  }
  it = message.begin();
  for (size_t i = 0, k = 0; i <= end; ++i, ++it)
  {
    if (*it == ' ' || i == end)
    {
      switch (k)
      {
        case 0:
          break;
        case 1:
          statuscode = ft_stoi(buffer);
          break;
        case 2:
          statusmessage.assign(buffer);
          break;
        default:
          throw (std::logic_error("startline ERROR"));
      }
      k++;
      buffer.clear();
    }
    else
    {
      buffer += *it;
    }
  }
  context->res = new HTTPResponse(statuscode, statusmessage, context->manager->getServerName(context->addr.sin_port));
  message.erase(0, end + 2);
}

void CGI::parseCGI(struct Context* context)
{
  std::string message;
  char buffer[BUFFER_SIZE] = {0};

  while (read(context->cgi->readFD, buffer, sizeof(buffer)))
  {
    message.append(buffer);
    memset(buffer, 0, sizeof(buffer));
  }
  close(context->cgi->readFD);
  parseStartLine(context, message);
  parseHeader(context->res, message);
  parseBody(context->res, message);
}

void CGI::CGIEvent(struct Context* context)
{
  struct Context* newContext = new struct Context(context->fd, context->addr, pipeWriteHandler, context->manager);
  newContext->req = context->req;
  newContext->cgi = context->cgi;
  struct kevent event;
  EV_SET(&event, context->cgi->writeFD, EVFILT_WRITE, EV_ADD, 0, 0, newContext);
  context->manager->attachNewEvent(newContext, event);
  delete context;
}

void CGI::addEnv(std::string key, std::string val)
{
  std::string temp;

  if (envCount >= ENVCOUNT)
  {
    return;
  }
  temp.assign(key);
  temp.append("=");
  temp.append(val);
  env[envCount] = new char[temp.size() + 1];
  temp.copy(this->env[envCount], temp.size());
  this->env[envCount][temp.size()] = '\0';
  this->env[envCount + 1] = NULL;
  envCount++;
}

std::string CGI::getQueryFullPath(HTTPRequest& req)
{
  std::map<std::string, std::string>::iterator it;
  std::string temp;

  for (it = req.query.begin(); it != req.query.end(); ++it)
  {
    temp.append(it->first);
    temp.append("=");
    temp.append(it->second);
    temp.append("&");
  }
  return (temp);
}
std::string CGI::getCWD()
{
  char buffer[PATH_MAX];
  std::string path;

  if (!getcwd(buffer, PATH_MAX))
  {
    throw (std::runtime_error("getcwd fail"));
  }
  path.assign(buffer);
  return (path);
}
//path다르게 들어오면 redirection?아니면 오류 처리?

void CGI::getPATH(Server server, HTTPRequest& req)
{
  std::string requestpath;
  std::string requestcmd;
  Location* location = server.getMatchedLocation(req);

  requestpath.assign(getCWD());
  requestpath.append(location->_root);
  requestpath.append("/");
  requestpath.append(location->cgiInfo[1]);
  requestpath.copy(path, requestpath.size() + 1);
  requestpath.copy(cmd[0], requestpath.size() + 1);
  path[requestpath.size()] = '\0';
  cmd[0][requestpath.size()] = '\0';
  if (location->cgiInfo.size() > 2)
  {
    requestcmd.assign(location->cgiInfo[2]);
    requestcmd.copy(cmd[1], requestcmd.size() + 1);
    cmd[1][requestcmd.size()] = '\0';
  }
  else
  {
    cmd[1] = NULL;
  }
  addEnv("PATH_INFO", requestpath);
  requestpath.append("?var1=value1&var2=with%20percent%20encoding");
  addEnv("REQUEST_URI", requestpath);
}

void CGI::setCGIenv(Server server, HTTPRequest& req, struct Context* context)
{
  addEnv("SERVER_SOFTWARE", "webserv/1.1");
  addEnv("SERVER_PROTOCOL", "HTTP/1.1");
  addEnv("SERVER_NAME", server._serverName);
  addEnv("GATEWAY_INTERFACE", "CGI/1.1");
  addEnv("SERVER_PORT", ft_itos(server._serverPort));
  addEnv("REQUEST_METHOD", getMethodType(req.method));
  addEnv("SCRIPT_NAME", "webserv/1.1");
  addEnv("QUERY_STRING", getQueryFullPath(req));
  addEnv("REMOTE_ADDR", getClientIP(&context->addr));
  addEnv("CONTENT_TYPE", req.headers.find("Content-Type")->second);
  addEnv("CONTENT_LENGTH", req.headers.find("Content-Length")->second);
  getPATH(server, req);
  for (size_t i = 0; i < ENVCOUNT; ++i)
  {
    if (env[i] == NULL)
      return;
    std::cout << env[i] << std::endl;
  }
}
// fork, pipe init
void CGI::processInit(CGI* cgi)
{
  int infd[2];
  int outfd[2];

  if (pipe(infd) == -1 || pipe(outfd) == -1)
  {
    throw (std::runtime_error("pipe make fail"));
  }
  printf("path : %s\n", cgi->path);
  printf("cmd : %s\n", cgi->cmd[0]);
  printf("null check : %p", cgi->cmd[1]);
  cgi->pid = fork();
  if (cgi->pid < 0)
  {
    throw (std::runtime_error("fork fail"));
  }
  if (cgi->pid == 0)
  {
    close(infd[P_W]);
    close(outfd[P_R]);
    dup2(outfd[P_W], STDOUT_FILENO);
    dup2(infd[P_R], STDIN_FILENO);
    if (execve(cgi->path, cgi->cmd, cgi->env) < 0)
    {
      std::cerr << errno;
      std::cerr << strerror(errno);
      exit (1);
    }
  }
  close(infd[P_R]);
  close(outfd[P_W]);
  cgi->writeFD = infd[P_W];
  cgi->readFD = outfd[P_R];
}

void CGIProcess(struct Context* context)
{
  context->cgi = new CGI();
  HTTPRequest& req = *context->req;
  Server& server = context->manager->getMatchedServer(req);

  context->cgi->setCGIenv(server, req, context);
  context->cgi->processInit(context->cgi);
  context->cgi->CGIEvent(context);
}

bool isCGIRequest(const std::string& file, Location* loc)
{
  size_t findPOS;

  if (loc->cgiInfo.size() == 0)
  {
    return false;
  }
  findPOS = file.find(*loc->cgiInfo.begin());
  if (findPOS == std::string::npos)
  {
    return false;
  }
  if (!file.compare(findPOS, std::string::npos, *loc->cgiInfo.begin()))
  {
    return true;
  }
  return false;
}