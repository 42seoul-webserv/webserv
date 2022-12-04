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
  //int fd[2];
  res->addHeader("Content-Length", ft_itos(100000000));
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
  {std::cerr << "1" << std::endl;
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
        {std::cerr << "2" << std::endl;
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
{close(context->cgi->readFD);
  context->cgi->readFD = open("../tempfile/out", O_RDONLY);
  std::string message;
  char buffer[BUFFER_SIZE] = {0};
  read(context->cgi->readFD, buffer, 58);
  message.append(buffer);
//  close(context->cgi->readFD);
  parseStartLine(context, message);

  parseHeader(context->res, message);

  parseBody(context->res, message);

  context->res->setFd(context->cgi->readFD);
}

void CGI::CGIFileWriteEvent(struct Context* context)
{
  struct Context* newContext = new struct Context(context->cgi->writeFD, context->addr, CGIWriteHandler, context->manager);
  newContext->req = context->req;
  struct kevent event;
  EV_SET(&event, newContext->fd, EVFILT_WRITE, EV_ADD, 0, 0, newContext);
  newContext->manager->attachNewEvent(newContext, event);
}

void CGI::CGIChildEvent(struct Context* context)
{
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
  int inFD;
  int outFD;
  int temp_in = dup(STDIN_FILENO);
	int temp_out = dup(STDOUT_FILENO);
  std::string infilepath;
  std::string outfilepath;

  infilepath.assign(getCWD());
  infilepath.append("/tempfile/");
  infilepath.append("in");
  outfilepath.assign(getCWD());
  outfilepath.append("/tempfile/");
  outfilepath.append("out");
 
  printf("path : %s\n", cgi->path);
  printf("cmd : %s\n", cgi->cmd[0]);
  printf("null check : %p", cgi->cmd[1]);
  inFD = open(infilepath.c_str(), O_RDWR | O_CREAT | O_TRUNC | O_NONBLOCK, 0777);
  outFD = open(outfilepath.c_str(), O_RDWR | O_CREAT | O_TRUNC | O_NONBLOCK, 0777);
  dup2(outFD, STDOUT_FILENO);
  dup2(inFD, STDIN_FILENO);
  cgi->pid = fork();
  if (cgi->pid < 0)
  {
    throw (std::runtime_error("fork fail"));
  }
  if (cgi->pid == 0)
  {
    if (execve(cgi->path, cgi->cmd, cgi->env) < 0)
    {
      std::cerr << errno;
      std::cerr << strerror(errno);
      exit (1);
    }
  }
  close(inFD);
  inFD = open(infilepath.c_str(), O_RDWR | O_CREAT | O_TRUNC | O_NONBLOCK, 0777);
  dup2(temp_out, STDOUT_FILENO);
  dup2(temp_in, STDIN_FILENO);
  cgi->writeFD = inFD;
  cgi->readFD = outFD;
}

void CGIProcess(struct Context* context)
{std::cerr<< "cgi ckeck" << std::endl;
  context->cgi = new CGI();
  HTTPRequest& req = *context->req;
  Server& server = context->manager->getMatchedServer(req);

  context->cgi->setCGIenv(server, req, context);
  context->cgi->processInit(context->cgi);
  context->cgi->CGIFileWriteEvent(context);
  context->cgi->CGIChildEvent(context);
}

bool isCGIRequest(const std::string& file, Location* loc)
{std::cerr <<"iscgi req" << std::endl;
std::cerr << "filepath : " << file << std::endl;
std::cerr << "loc : " << loc->_location << std::endl;
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