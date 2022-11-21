#include "Parser.hpp"
#include "Server.hpp"
#include "WebservDefines.hpp"
#include <cctype>
#include <sstream>

//CommonParser
bool CommonParser::IsNodeElemEmpty(ParserNode node)
{
  if (node.elem.empty())
  {
    return (true);
  }
  return (false);
}

ParserNode *CommonParser::GetNextNode(ParserNode node)
{
  return (node.next);
}

ParserNode *ConfigParser::getNode(size_t server_index,
                                  const std::string &category)
{
  ParserNode *temp = NULL;

  temp = _nodeVector[server_index].next;
  while (temp && temp->category != category)
  {
    temp = temp->next;
  }
  return (temp);
}

std::vector<std::string> ConfigParser::GetNodeElem(size_t server_index,
                                                   const std::string &category,
                                                   const std::string &key)
{
  ParserNode *temp = NULL;
  std::vector<std::string> empty;
  std::map<std::string, std::vector<std::string> >::iterator it;

  temp = _nodeVector[server_index].next;
  while (temp && temp->category != category)
  {
    temp = temp->next;
  }
  if (temp)
  {
    it = temp->elem.find(key);
  }
  if (temp == NULL || it == temp->elem.end())
  {
    empty.resize(1);
    return (empty);
  }
  return (it->second);
}

void CommonParser::displayAll()
{
  for (unsigned int i = 0; i < _nodeVector.size(); ++i)
  {
    ParserNode *temp = &_nodeVector[i];
    while (temp)
    {
      std::cout << temp->category << std::endl;
      for (std::map<std::string, std::vector<std::string> >::iterator it = temp->elem.begin(); \
                it != temp->elem.end(); ++it)
      {
        std::cout << "  " << it->first << " : ";
        for (unsigned long secondIdx = 0;
             secondIdx < it->second.size(); ++secondIdx)
        {
          std::cout << it->second[secondIdx] << " ";
        }
        std::cout << std::endl;
      }
      temp = temp->next;
    }
  }
}

//ConfigParser
bool ConfigParser::vaildCheck(const std::string &FileRoot)
{
  std::stack<char> checkStack;
  std::filebuf fb;
  char getChar;

  if (fb.open(FileRoot, std::ios::in) == NULL)
  {
    return (false);
  }
  std::istream is(&fb);
  while (is)
  {
    getChar = static_cast<char>(is.get());
    if (getChar == '{')
    {
      checkStack.push(getChar);
    }
    else if (getChar == '}')
    {
      if (checkStack.top() == '{')
      {
        checkStack.pop();
      }
      else
      {
        checkStack.push(getChar);
      }
    }
  }
  fb.close();
  if (checkStack.empty())
  {
    return (true);
  }
  return (false);
}

// private
void ConfigParser::getElem(ParserNode *temp, const std::string &line)
{
  std::string key;
  std::string buffer;
  std::vector<std::string> value;

  for (std::string::const_iterator it = line.begin(); it != line.end(); ++it)
  {
    if (isblank(*it) || *it == ';')
    {
      if (!key.empty() && !buffer.empty())
      {
        value.push_back(buffer);
        buffer.clear();
      }
      continue;
    }
    if (*it == ':' && key.empty())
    {
      key = buffer;
      buffer.clear();
    }
    else
    {
      buffer += *it;
    }
  }
  temp->elem[key] = value;
}

ParserNode *ConfigParser::EnterNode(ParserNode *temp, const std::string &line)
{
  while (temp->next)
  {
    temp = temp->next;
  }
  temp->next = new ParserNode;
  temp->next->prev = temp;
  temp->next->next = NULL;
  temp = temp->next;
  for (std::string::const_iterator it = line.begin(); it != line.end(); ++it)
  {
    if ((isblank(*it) && *(it + 1) != '/') || *it == '{')
    {
      continue;
    }
    temp->category += *it;
  }
  return (temp);
}

CommonParser::~CommonParser()
{
  ParserNode *origin;
  ParserNode *temp;

  for (unsigned int i = 0; i < _nodeVector.size(); ++i)
  {
    origin = _nodeVector[i].next;
    while (origin)
    {
      temp = origin->next;
      delete (origin);
      origin = temp;
    }
  }
}

void ConfigParser::parsingOneNode(std::istream &is)
{
  std::string buffer;
  ParserNode rootNode;
  ParserNode *temp = &rootNode;

  rootNode.next = NULL;
  rootNode.prev = NULL;
  while (!getline(is, buffer).eof())
  {
    if (buffer.empty())
    {
      continue;
    }
    if (*buffer.rbegin() == ';')
    {
      getElem(temp, buffer);
    }
    else if (*buffer.rbegin() == '{')
    {
      temp = EnterNode(temp, buffer);
    }
    else if (*buffer.rbegin() == '}')
    {
      if (temp == rootNode.next)
      {
        break;
      }
      temp = rootNode.next;
    }
    buffer.clear();
  }
  if (rootNode.next)
  {
    _nodeVector.push_back(rootNode);
  }
}

void ConfigParser::getAllowMethods(std::vector<MethodType> &_allowMethods,
                                   const std::string &category,
                                   unsigned int server_index)
{
  std::vector<std::string> methods;

  methods = GetNodeElem(server_index, category, "allow_methods");
  for (unsigned int i = 0; i < methods.size(); ++i)
  {
    MethodType method = getMethodType(methods[i]);
    _allowMethods.push_back(method);
  }
  if (_allowMethods.empty())
    _allowMethods.push_back(DEFAULT_ALLOW_METHODS);
}

static void setLocationDefault(Server &server, Location &location)
{
  if (location.root.empty())
  {
    location.root = server._root;
  }
  if (location.index.empty())
  {
    location.index = server._index;
  }
}

void ConfigParser::getLocationAttr(Server &server, unsigned int server_index)
{
  size_t found;
  std::string temp_cate;
  std::vector<std::string> temp2;
  Location location;

  for (ParserNode *temp = _nodeVector[server_index].next;
       temp != NULL; temp = temp->next)
  {
    found = temp->category.find("location ");
    if (found != std::string::npos)
    {
      temp_cate = temp->category;
      temp_cate.erase(temp_cate.begin(), temp_cate.begin() + 9);
      location.location = temp_cate;
      if (location.location.empty())
      {
        throw (std::runtime_error("invalid config file\n"));
      }
      std::string indexString = *(GetNodeElem(server_index,
                                              temp->category,
                                              "index").begin());
      if (indexString.empty())
        location.index = DEFAULT_INDEX;

      std::string rootString = *(GetNodeElem(server_index,
                                    temp->category,
                                    "root").begin());
      if (rootString.empty())
        location.root = DEFAULT_ROOT;
      getAllowMethods(location.allowMethods, temp->category, server_index);
      if (!GetNodeElem(server_index,
                       temp->category,
                       "client_max_body_size").begin()->empty())
      {
        location.clientMaxBodySize = ft_stoi(*(GetNodeElem(server_index,
                                                           temp->category,
                                                           "client_max_body_size").begin()));
      } // FIXME crash 가능성
      else
        location.clientMaxBodySize = DEFAULT_CLIENT_MAX_BODY_SIZE;
      location.cgiInfo = GetNodeElem(server_index, temp->category, "cgi_info");
      setLocationDefault(server, location);
      server._locations.push_back(location);
      location.allowMethods.clear();
      location.cgiInfo.clear();
    }
  }
}

void ConfigParser::displayServer(Server &server)
{
  std::cout << "server allowMethods : ";
  for (unsigned int i = 0; i < server._allowMethods.size(); ++i)
  {
    std::cout << server._allowMethods[i] << " ";
  }
  std::cout << std::endl;
  std::cout << "error_page : " << std::endl;
  for (std::map<StatusCode, std::string>::iterator it = server._errorPage.begin();
       it != server._errorPage.end(); ++it)
  {
    std::cout << it->first << " : ";
    std::cout << it->second << std::endl;
  }
  std::cout << std::endl;
  for (unsigned int i = 0; i < server._locations.size(); ++i)
  {
    std::cout << "location : " << server._locations[i].location << std::endl;
    std::cout << "index : " << server._locations[i].index << std::endl;
    std::cout << "root : " << server._locations[i].root << std::endl;
    std::cout << "maxsize : " << server._locations[i].clientMaxBodySize
              << std::endl;
    std::cout << "location allow methods: ";
    for (unsigned int k = 0; k < server._locations[i].allowMethods.size(); ++k)
    {
      std::cout << server._locations[i].allowMethods[k] << " ";
    }
    std::cout << std::endl;
    std::cout << "cgiInfo : ";
    for (unsigned int k = 0; k < server._locations[i].cgiInfo.size(); ++k)
    {
      std::cout << server._locations[i].cgiInfo[k] << " ";
    }
    std::cout << std::endl;
  }
}

void ConfigParser::getErrorPage(std::map<StatusCode, std::string> &_errorPage,
                                unsigned int server_index)
{
  ParserNode *errorPageNode = getNode(server_index, "error_page");
  std::map<std::string, std::vector<std::string> >::iterator it;

  if (errorPageNode)
  {
    for (it = errorPageNode->elem.begin();
         it != errorPageNode->elem.end(); ++it)
    {
      _errorPage[static_cast<StatusCode>(std::stod(it->first))] = *(it->second.begin());
    }
  }
}

//begin empty일때
void ConfigParser::getServerAttr(Server &server, unsigned int server_index)
{
  std::string listenAddress;
  std::string serverListenIP;
  int serverListenPort;

  // set server ip
  listenAddress = *(GetNodeElem(server_index, "server", "listen").begin());
  if (listenAddress.empty())
    listenAddress = DEFAULT_SOCKET_LISTEN_ADDR;
  serverListenIP = listenAddress.substr(0, listenAddress.find(':'));
  inet_pton(AF_INET, serverListenIP.c_str(), &server._socketAddr.sin_addr);
  // ser server port
  std::string portString = listenAddress.substr(listenAddress.find(':') + 1);
  if (!portString.empty())
    serverListenPort = ft_stoi(portString);
  else
    serverListenPort = DEFAULT_SERVER_PORT;
  server._socketAddr.sin_port = ntohs(serverListenPort);
  server._serverPort = ntohs(serverListenPort);
  server._socketAddr.sin_family = AF_INET;

  // set server config
  server._index = *(GetNodeElem(server_index, "server", "index").begin());
  if (server._index.empty())
  {
    server._index = DEFAULT_INDEX;
  }
  server._root = *(GetNodeElem(server_index, "server", "root").begin());
  if (server._root.empty())
  {
    server._serverName = DEFAULT_ROOT;
  }
  server._serverName = *(GetNodeElem(server_index,
                                     "server",
                                     "server_name").begin());
  if (server._serverName.empty())
  {
    server._serverName = DEFAULT_SERVER_NAME;
  }
  std::string clientMaxBodySize = *(GetNodeElem(server_index,
                                                "server",
                                                "client_max_body_size").begin());
  if (clientMaxBodySize.empty())
  {
    server._clientMaxBodySize = DEFAULT_CLIENT_MAX_BODY_SIZE;
  }
  server._clientMaxBodySize = ft_stoi(clientMaxBodySize);
  getAllowMethods(server._allowMethods, "server", server_index);
  if (server._allowMethods.empty())
  {
    server._allowMethods.push_back(DEFAULT_ALLOW_METHODS);
  }
  getLocationAttr(server, server_index);
  getErrorPage(server._errorPage, server_index);
  displayServer(server);
}

std::vector<Server> ConfigParser::parsing(const std::string &FileRoot)
{
  std::filebuf fb;
  std::vector<Server> _serverList;

  if (fb.open(FileRoot, std::ios::in) == NULL)
  {
    throw (std::runtime_error("open fail\n"));
  }
  std::istream is(&fb);
  while (is)
  {
    parsingOneNode(is);
  }
  fb.close();
  _serverList.resize(_nodeVector.size());

  const unsigned int SIZE = _nodeVector.size();
  for (unsigned int i = 0; i < SIZE; ++i)
  {
    getServerAttr(_serverList[i], i);
  }
  return (_serverList);
}