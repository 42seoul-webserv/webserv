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

void CGI::parseBody(HTTPResponse* res, size_t count)
{
  //int fd[2];
  res->addHeader("Content-Length", ft_itos(FdGetFileSize(readFD) - count));
  std::cerr << res->getHeader().toString() <<std::endl;
  //res->addHeader("Content-Length", ft_itos(message.size()));
 /* if (pipe(fd) < 0)
  {
    throw (std::runtime_error("cgi pipe boom"));
  }
  write(fd[P_W], message.c_str(), message.size());
  close(fd[P_W]);*/
  //res->setFd(fd[P_R]);
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
        {
          throw (std::logic_error("startline ERROR"));
        }
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
  context->cgi->readFD = open(readFilePath.c_str(), O_RDONLY, 0777);
  std::string message;
  char buffer[1];
  size_t count;
  while (read(context->cgi->readFD, buffer, 1))
  {
    message.append(buffer, 1);
    if (message.find("\r\n\r\n") != std::string::npos)
    {
      break;
    }
  }
  count = message.size();
  parseStartLine(context, message);
  parseHeader(context->res, message);
  parseBody(context->res, count);
  context->res->setFd(context->cgi->readFD);
}

void CGI::CGIFileWriteEvent(struct Context* context)
{
  context->cgi->writeFD = open(context->cgi->writeFilePath.c_str(),  O_WRONLY | O_CREAT | O_TRUNC, 0777);
  struct Context* newContext = new struct Context(context->fd, context->addr, CGIWriteHandler, context->manager);
  newContext->req = context->req;
  newContext->cgi = context->cgi;
  newContext->connectContexts = context->connectContexts;
  struct kevent event;
  EV_SET(&event, newContext->cgi->writeFD, EVFILT_WRITE, EV_ADD, 0, 0, newContext);
  newContext->manager->attachNewEvent(newContext, event);
}

void CGI::CGIfork(struct Context* context)
{
  int inFD;
  int outFD;

  context->cgi->pid = fork();
  if (context->cgi->pid < 0)
  {
    throw (std::runtime_error("fork fail"));
  }
  if (context->cgi->pid == 0)
  {
    inFD = open(writeFilePath.c_str(), O_RDONLY, 0777);
    outFD = open(readFilePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC , 0777);
    dup2(outFD, STDOUT_FILENO);
    dup2(inFD, STDIN_FILENO);
    close(outFD);
    close(inFD);  
    if (execve(context->cgi->path, context->cgi->cmd, context->cgi->env) < 0)
    {
      std::cerr << errno;
      std::cerr << strerror(errno);
      exit (1);
    }
      std::cerr << errno;
      std::cerr << strerror(errno);
      exit (2);
  }
}

void CGI::CGIChildEvent(struct Context* context)
{
  context->cgi->CGIfork(context);
  struct Context* newContext = new struct Context(context->fd, context->addr, CGIChildHandler, context->manager);
  newContext->cgi = context->cgi;
  newContext->req = context->req;
  newContext->connectContexts = context->connectContexts;
  struct kevent event;
  EV_SET(&event, newContext->cgi->pid, EVFILT_PROC, EV_ADD | EV_ENABLE, NOTE_EXIT | NOTE_EXITSTATUS, newContext->cgi->exitStatus, newContext);
  newContext->manager->attachNewEvent(newContext, event);
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
  path.erase(path.find("/build"));
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
  addEnv("REQUEST_METHOD", methodToString(req.method));
  addEnv("SCRIPT_NAME", "webserv/1.1");
  addEnv("QUERY_STRING", getQueryFullPath(req));
  addEnv("REMOTE_ADDR", getClientIP(&context->addr));
  addEnv("CONTENT_TYPE", req.headers.find("Content-Type")->second);
  if (req.headers.find("Content-Length") == req.headers.end())
  {
    addEnv("CONTENT_LENGTH", "");
  }
  else
    addEnv("CONTENT_LENGTH", req.headers.find("Content-Length")->second);
  getPATH(server, req);
}
// fork, pipe init
void CGI::processInit(CGI* cgi)
{
  std::string infilepath;
  std::string outfilepath;
  static size_t fileCount;

  infilepath.assign(getCWD());
  infilepath.append("/tempfile/in");
  infilepath.append(ft_itos(fileCount));
  outfilepath.assign(getCWD());
  outfilepath.append("/tempfile/out");
  outfilepath.append(ft_itos(fileCount++));
  writeFilePath = infilepath;
  readFilePath = outfilepath;
}

void CGIProcess(struct Context* context)
{
  context->cgi = new CGI();
  HTTPRequest& req = *context->req;
  Server& server = context->manager->getMatchedServer(req);

  context->cgi->setCGIenv(server, req, context);
  context->cgi->processInit(context->cgi);
  context->cgi->CGIFileWriteEvent(context);
  //context->cgi->CGIChildEvent(context);
}

bool isCGIRequest(const std::string& file, Location* loc)
{
  size_t findPOS;

  if (loc->cgiInfo.size() == 0)
  {
    return false;
  }
  else
    return true;
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