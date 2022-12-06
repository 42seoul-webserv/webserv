#include "CGI.hpp"
#include <cctype>
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
  res->addHeader("Content-Length", ft_itos(FdGetFileSize(readFD) - count));
//  std::cerr << std::endl;
//  std::cerr << res->getHeader().toString() <<std::endl;
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

void CGI::parseCGI(struct Context* context, std::string& message)
{
  size_t count;
  count = message.size();
  parseStartLine(context, message);
  parseHeader(context->res, message);
  parseBody(context->res, count);
  context->res->setFd(context->cgi->readFD);
}

void CGI::attachFileWriteEvent(struct Context* context)
{
  context->cgi->writeFD = open(context->cgi->writeFilePath.c_str(),  O_WRONLY | O_CREAT | O_TRUNC, 0777);
  struct Context* newContext = new struct Context(context->fd, context->addr, CGIWriteHandler, context->manager);
  newContext->req = context->req;
  newContext->cgi = context->cgi;
  newContext->threadKQ = context->threadKQ;
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
      std::cerr << strerror(errno);
      exit (1);
    }
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
std::string CGI::ft_getcwd()
{
  char buffer[PATH_MAX];
  std::string path;

  if (!getcwd(buffer, PATH_MAX))
  {
    throw (std::runtime_error("getcwd fail"));
  }
  path.assign(buffer);
  path.erase(path.find("/build"));// 나중에 고쳐야하나
  return (path);
}
void CGI::getPATH(Server server, HTTPRequest& req)
{
  std::string requestpath;
  std::string requestcmd;
  Location* location = server.getMatchedLocation(req);

  requestpath.assign(ft_getcwd());
  requestpath.append(location->_root);
  requestpath.append("/");
  if (location->cgiInfo.size() == 2)
  {
    requestpath.append(location->cgiInfo[1]);
    requestpath.copy(path, requestpath.size() + 1);
    requestpath.copy(cmd[0], requestpath.size() + 1);
    path[requestpath.size()] = '\0';
    cmd[0][requestpath.size()] = '\0';
    cmd[1] = NULL;
    addEnv("PATH_INFO", requestpath);
    requestpath.append(encodePercentEncoding(getQueryFullPath(req)));
    addEnv("REQUEST_URI", requestpath);
  }
  else
  {
    requestpath.append(location->cgiInfo[2]);
    requestpath.copy(cmd[1], requestpath.size() + 1);
    cmd[1][requestpath.size()] = '\0';
    requestcmd.assign(location->cgiInfo[1]);
    requestcmd.copy(path, requestcmd.size() + 1);
    path[requestcmd.size()] = '\0';
    requestcmd.copy(cmd[0], requestcmd.size() + 1);
    cmd[0][requestcmd.size()] = '\0';
    addEnv("PATH_INFO", requestcmd);
    requestcmd.append(encodePercentEncoding(getQueryFullPath(req)));
    addEnv("REQUEST_URI", requestcmd);
  }
}

void CGI::setRequestEnv(HTTPRequest& req)
{
  std::string schema = "HTTP_";
  std::string key;
  std::string val;
  char c;

  for (std::map<std::string, std::string>::const_iterator it = req.headers.begin();\
        it != req.headers.end(); ++it)
  {
    //transfer(it->frist,key) &key assign
    key.assign(schema);
    for (size_t i = 0; i < it->first.size(); ++i)
    {
      c = it->first[i];
      if (islower(c))
      {
        c = toupper(c);
      }
      else if (c == '-')
      {
        c = '_';
      }
      key.append(&c, 1);
    }
    val.assign(it->second);
    addEnv(key, val);
  }
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
  addEnv("QUERY_STRING", encodePercentEncoding(getQueryFullPath(req)));
  addEnv("REMOTE_ADDR", getClientIP(&context->addr));
  addEnv("CONTENT_TYPE", req.headers.find("Content-Type")->second);
  getPATH(server, req);
  setRequestEnv(req);
/*  for (size_t i = 0; i < envCount; ++i)
  {
    std::cerr << env[i]<< std::endl;
  }*/
}
// fork, pipe init

void CGI::setFilePath(CGI* cgi)
{
  std::string infilepath;
  std::string outfilepath;
  static size_t fileCount;

  infilepath.assign(ft_getcwd());
  infilepath.append("/tempfile/in");
  infilepath.append(ft_itos(fileCount));
  outfilepath.assign(ft_getcwd());
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
  context->cgi->setFilePath(context->cgi);
  context->cgi->attachFileWriteEvent(context);
}

bool isCGIRequest(const std::string& file, Location* loc)
{
  size_t findPOS;

  if (loc->cgiInfo.begin()->size() == 0)
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