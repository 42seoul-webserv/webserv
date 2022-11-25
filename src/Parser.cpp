#include "Parser.hpp"
#include "Server.hpp"
#include "WebservDefines.hpp"
#include <cctype>
#include <sstream>

//CommonParser
bool CommonParser::isNodeElementEmpty(ParserNode node)
{
  if (node.elem.empty())
  {
    return (true);
  }
  return (false);
}

ParserNode* CommonParser::getNextNode(ParserNode node)
{
  return (node.next);
}

ParserNode* ConfigParser::getNode(size_t server_index,
                                  const std::string& category)
{
  ParserNode* temp = NULL;

  temp = _nodeVector[server_index].next;
  while (temp && temp->category != category)
  {
    temp = temp->next;
  }
  return (temp);
}

std::vector<std::string> ConfigParser::GetNodeElem(size_t serverIndex,
                                                   const std::string& category,
                                                   const std::string& key)
{
  ParserNode* temp = NULL;
  std::vector<std::string> empty;
  std::map<std::string, std::vector<std::string> >::iterator it;

  temp = _nodeVector[serverIndex].next;
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
  for (
          unsigned int i = 0; i < _nodeVector.size(); ++i
          )
  {
    ParserNode* temp = &_nodeVector[i];
    while (temp)
    {
      std::cout << temp->category << std::endl;
      for (
              std::map<std::string, std::vector<std::string> >::iterator it = temp->elem.begin(); \
                it != temp->elem.end(); ++it
              )
      {
        std::cout << "  " << it->first << " : ";
        for (
                unsigned long secondIdx = 0;
                secondIdx < it->second.size(); ++secondIdx
                )
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
bool ConfigParser::isValidFile(const std::string& configFilePath)
{
  std::stack<char> checkStack;
  std::filebuf fb;
  char getChar;

  if (fb.open(configFilePath, std::ios::in) == NULL)
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
void ConfigParser::getElem(ParserNode* temp, const std::string& line)
{
  std::string key;
  std::string buffer;
  std::vector<std::string> value;

  for (
          std::string::const_iterator it = line.begin(); it != line.end(); ++it
          )
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

ParserNode* ConfigParser::enterNode(ParserNode* temp, const std::string& line)
{
  while (temp->next)
  {
    temp = temp->next;
  }
  temp->next = new ParserNode;
  temp->next->prev = temp;
  temp->next->next = NULL;
  temp = temp->next;
  for (
          std::string::const_iterator it = line.begin(); it != line.end(); ++it
          )
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
  ParserNode* origin;
  ParserNode* temp;

  for (
          unsigned int i = 0; i < _nodeVector.size(); ++i
          )
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

void ConfigParser::parseOneNode(std::istream& is)
{
  std::string buffer;
  ParserNode rootNode;
  ParserNode* temp = &rootNode;

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
      temp = enterNode(temp, buffer);
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

void ConfigParser::getAllowMethods(std::vector<MethodType>& allowMethods,
                                   const std::string& category,
                                   unsigned int serverIndex)
{
  std::vector<std::string> methods;

  methods = GetNodeElem(serverIndex, category, "allow_methods");
  for (
          unsigned int i = 0; i < methods.size(); ++i
          )
  {
    MethodType method = getMethodType(methods[i]);
    allowMethods.push_back(method);
  }
  if (allowMethods.empty())
  {
    allowMethods.push_back(DEFAULT_ALLOW_METHODS);
  }
}

static void setLocationDefault(Server& server, Location& location)
{
  if (location._root.empty())
  {
    location._root = server._root;
  }
  if (location._index.empty())
  {
    location._index = server._index;
  }
}

void ConfigParser::getLocationAttr(Server& server, unsigned int serverIndex) {
  size_t found;
  std::string temp_cate;
  std::vector<std::string> temp2;
  Location location;

  for (
          ParserNode *temp = _nodeVector[serverIndex].next;
          temp != NULL; temp = temp->next
          ) {
    found = temp->category.find("location ");
    if (found != std::string::npos) {
      temp_cate = temp->category;
      temp_cate.erase(temp_cate.begin(), temp_cate.begin() + 9);
      location._location = temp_cate;
      if (location._location.empty()) {
        throw (std::runtime_error("invalid config file\n"));
      }
      std::string indexString = *(GetNodeElem(serverIndex,
                                              temp->category,
                                              "index").begin());
      location._index = indexString;
      if (indexString.empty()) {
        location._index = DEFAULT_INDEX;
      }

      std::string rootString = *(GetNodeElem(serverIndex,
                                             temp->category,
                                             "root").begin());
      location._root = rootString;
      if (rootString.empty()) {
        location._root = DEFAULT_ROOT;
      }
      getAllowMethods(location.allowMethods, temp->category, serverIndex);


      std::vector<std::string> redirects = GetNodeElem(serverIndex, temp->category, "redirect");
      // if has redirection
      if (!redirects.empty() && redirects.size() == 2) {
        std::vector<std::string>::const_iterator itr = redirects.begin();
        location._redirect.first = static_cast<StatusCode>(std::stod(*itr));
        itr++;
        location._redirect.second = *itr;
      }

        if (!GetNodeElem(serverIndex,
                         temp->category,
                         "client_max_body_size").begin()->empty()) {
          location.clientMaxBodySize = ft_stoi(*(GetNodeElem(serverIndex,
                                                             temp->category,
                                                             "client_max_body_size").begin()));
        } // FIXME crash 가능성
        else {
          location.clientMaxBodySize = DEFAULT_CLIENT_MAX_BODY_SIZE;
        }
        location.cgiInfo = GetNodeElem(serverIndex, temp->category, "cgi_info");
        setLocationDefault(server, location);
        server._locations.push_back(location);
        location.allowMethods.clear();
        location.cgiInfo.clear();
      }
    }
  }

void ConfigParser::displayServer(Server& server)
{
  std::cout << "server allowMethods : ";
  for (
          unsigned int i = 0; i < server._allowMethods.size(); ++i
          )
  {
    std::cout << server._allowMethods[i] << " ";
  }
  std::cout << std::endl;
  std::cout << "redirct code: " << server._redirect.first << std::endl;
  std::cout << "redirct url: " << server._redirect.second << std::endl;
  std::cout << "error_page : " << std::endl;
  for (
          std::map<StatusCode, std::string>::iterator it = server._errorPage.begin();
          it != server._errorPage.end(); ++it
          )
  {
    std::cout << it->first << " : ";
    std::cout << it->second << std::endl;
  }
  std::cout << std::endl;
//  std::cout << server._locations.size() << std::endl;
  for (
          unsigned int i = 0; i < server._locations.size(); ++i
          )
  {
    std::cout << "_location : " << server._locations[i]._location << std::endl;
    std::cout << "_index : " << server._locations[i]._index << std::endl;
    std::cout << "_root : " << server._locations[i]._root << std::endl;
    std::cout << "_redirct code: " << server._locations[i]._redirect.first << std::endl;
    std::cout << "_redirct url: " << server._locations[i]._redirect.second << std::endl;
    std::cout << "_maxsize : " << server._locations[i].clientMaxBodySize
              << std::endl;
    std::cout << "_location allow methods: ";
    for (
            unsigned int k = 0; k < server._locations[i].allowMethods.size(); ++k
            )
    {
      std::cout << server._locations[i].allowMethods[k] << " ";
    }
    std::cout << std::endl;
    std::cout << "_cgiInfo : ";
    for (
            unsigned int k = 0; k < server._locations[i].cgiInfo.size(); ++k
            )
    {
      std::cout << server._locations[i].cgiInfo[k] << " ";
    }
    std::cout << "\n" << std::endl;
  }
}

void ConfigParser::getErrorPage(std::map<StatusCode, std::string>& _errorPage,
                                unsigned int serverIndex)
{
  ParserNode* errorPageNode = getNode(serverIndex, "error_page");
  std::map<std::string, std::vector<std::string> >::iterator it;

  if (errorPageNode)
  {
    for (
            it = errorPageNode->elem.begin();
            it != errorPageNode->elem.end(); ++it
            )
    {
      _errorPage[static_cast<StatusCode>(std::stod(it->first))] = *(it->second.begin());
    }
  }
}

void ConfigParser::getRedirect(Server &server, unsigned int serverIndex)
{
  std::vector<std::string> redirects = GetNodeElem(serverIndex, "server", "redirect");
  if (!redirects.empty() && redirects.size() == 2)
  {
    std::vector<std::string>::const_iterator itr = redirects.begin();
    server._redirect.first = static_cast<StatusCode>(std::stod(*itr)) ;
    itr++;
    server._redirect.second = *itr;
  }
}

//begin empty일때
void ConfigParser::getServerAttr(Server& server, unsigned int serverIndex)
{
  std::string listenAddress;
  std::string serverListenIP;
  int serverListenPort;

  // set server ip
  listenAddress = *(GetNodeElem(serverIndex, "server", "listen").begin());
  if (listenAddress.empty())
  {
    listenAddress = DEFAULT_SOCKET_LISTEN_ADDR;
  }
  serverListenIP = listenAddress.substr(0, listenAddress.find(':'));
  inet_pton(AF_INET, serverListenIP.c_str(), &server._socketAddr.sin_addr);
  // ser server port
  std::string portString = listenAddress.substr(listenAddress.find(':') + 1);
  if (!portString.empty())
  {
    serverListenPort = ft_stoi(portString);
  }
  else
  {
    serverListenPort = DEFAULT_SERVER_PORT;
  }
  server._socketAddr.sin_port = ntohs(serverListenPort);
  server._serverPort = (serverListenPort);
  server._socketAddr.sin_family = AF_INET;

  // set server config
  server._index = *(GetNodeElem(serverIndex, "server", "index").begin());
  if (server._index.empty())
  {
    server._index = DEFAULT_INDEX;
  }
  server._root = *(GetNodeElem(serverIndex, "server", "root").begin());
  if (server._root.empty())
  {
    server._serverName = DEFAULT_ROOT;
  }
  server._serverName = *(GetNodeElem(serverIndex,
                                     "server",
                                     "server_name").begin());
  if (server._serverName.empty())
  {
    server._serverName = DEFAULT_SERVER_NAME;
  }
  std::string clientMaxBodySize = *(GetNodeElem(serverIndex,
                                                "server",
                                                "client_max_body_size").begin());
  if (clientMaxBodySize.empty())
  {
    server._clientMaxBodySize = DEFAULT_CLIENT_MAX_BODY_SIZE;
  }
  server._clientMaxBodySize = ft_stoi(clientMaxBodySize);
  getAllowMethods(server._allowMethods, "server", serverIndex);
  if (server._allowMethods.empty())
  {
    server._allowMethods.push_back(DEFAULT_ALLOW_METHODS);
  }
  getRedirect(server, serverIndex);
  getLocationAttr(server, serverIndex);
  getErrorPage(server._errorPage, serverIndex);

  displayServer(server);
}

std::vector<Server> ConfigParser::parseConfigFile(const std::string& configFilePath)
{
  std::filebuf fb;
  std::vector<Server> _serverList;

  if (fb.open(configFilePath, std::ios::in) == NULL)
  {
    throw (std::runtime_error("open fail\n"));
  }
  std::istream is(&fb);
  while (is)
  {
    parseOneNode(is);
  }
  fb.close();
  _serverList.resize(_nodeVector.size());

  const unsigned int SIZE = _nodeVector.size();
  for (
          unsigned int i = 0; i < SIZE; ++i
          )
  {
    getServerAttr(_serverList[i], i);
  }
  return (_serverList);
}