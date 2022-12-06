#ifndef PARSER_HPP
#define PARSER_HPP

#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stack>
#include <exception>
#include "Server.hpp"

#define DEFAULT_CLIENT_MAX_BODY_SIZE 10000
#define DEFAULT_SERVER_NAME "127.0.0.1"
#define DEFAULT_SERVER_PORT 80
#define DEFAULT_INDEX "_index.html"
#define DEFAULT_ROOT "./"
#define DEFAULT_SOCKET_LISTEN_ADDR "0.0.0.0:80"
#define DEFAULT_ALLOW_METHODS UNDEFINED

struct ParserNode
{
    ParserNode* next;
    ParserNode* prev;
    std::string category;
    std::map<std::string, std::vector<std::string> > elem;
};

class CommonParser
{
protected:
    std::vector<ParserNode> _nodeVector;
public:
    bool isNodeElementEmpty(ParserNode node);
    ParserNode* getNextNode(ParserNode node);
    ~CommonParser();
    void displayAll();
};

class ConfigParser : public CommonParser
{
private:
    void getElem(ParserNode* temp, const std::string& line);
    ParserNode* enterNode(ParserNode* temp, const std::string& line);
    void parseOneNode(std::istream& is);
    void getAllowMethods(std::vector<MethodType>& allowMethods,
                         const std::string& category,
                         unsigned int serverIndex);
    void getServerAttr(Server& server, unsigned int serverIndex);
    void getRedirect(Server& server, unsigned int serverIndex);
    void getLocationAttr(Server& server, unsigned int serverIndex);
    void displayServer(Server& server);
    void getErrorPage(std::map<StatusCode, std::string>& _errorPage,
                      unsigned int serverIndex);

public:
    std::vector<std::string> GetNodeElem(size_t serverIndex,
                                         const std::string& category,
                                         const std::string& key);
    ParserNode* getNode(size_t server_index, const std::string& category);
    static bool isValidFile(const std::string& configFilePath);
    std::vector<Server> parseConfigFile(const std::string& configFilePath);
};

#endif