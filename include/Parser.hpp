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
#define DEFAULT_INDEX "index.html"
#define DEFAULT_ROOT "./"
#define DEFAULT_SOCKET_LISTEN_ADDR "0.0.0.0:80"
#define DEFAULT_ALLOW_METHODS GET

struct ParserNode
{
    ParserNode *next;
    ParserNode *prev;
    std::string category;
    std::map<std::string, std::vector<std::string> > elem;
};

class CommonParser
{
protected:
    std::vector<ParserNode> _nodeVector;
public:
    bool IsNodeElemEmpty(ParserNode node);
    ParserNode* GetNextNode(ParserNode node);
    ~CommonParser();
    void displayAll();
};

class ConfigParser : public CommonParser
{
private:
    void getElem(ParserNode* temp, const std::string& line);
    ParserNode* EnterNode(ParserNode* temp, const std::string& line);
    void parsingOneNode(std::istream& is);
    void getAllowMethods(std::vector<MethodType>& _allowMethods, const std::string& category, unsigned int server_index);
    void getServerAttr(Server& server, unsigned int server_index);
    void getLocationAttr(Server& server, unsigned int server_index);
    void displayServer(Server& server);
    void getErrorPage(std::map<StatusCode, std::string>& _errorPage, unsigned int server_index);
public:
    std::vector<std::string> GetNodeElem(size_t server_index, const std::string& category , const std::string& key);
    ParserNode* getNode(size_t server_index, const std::string& category);
    static bool vaildCheck(const std::string& FileRoot);
    std::vector<Server> parsing(const std::string& FileRoot);
};