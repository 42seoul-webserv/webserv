#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stack>
#include <exception>
#include "Server.hpp"

struct ParserNode
{
    ParserNode *next;
    ParserNode *prev;
    std::string categoly;
    std::map<std::string, std::vector<std::string> > elem;
};

class CommonParser
{
protected:
    std::vector<ParserNode> nodevector;
public:
    bool IsNodeElemEmpty(ParserNode node);
    ParserNode* GetNextNode(ParserNode node);
    ~CommonParser();
    void displayAll();
};

class ConfigParser : public CommonParser
{
private:
    void getElem(ParserNode* temp, std::string line);
    ParserNode* EnterNode(ParserNode* temp, std::string line);
    void parsingOneNode(std::istream& is);
    void getAllowMethods(std::vector<MethodType>& _allowMethods, std::string categoly, unsigned int server_index);
    void getServerAttr(Server& server, unsigned int server_index);
    void getLocationAttr(Server& server, unsigned int server_index);
    void displayServer(Server& server);
public:
    std::vector<std::string> GetNodeElem(size_t server_index, std::string categoly ,std::string key);
    static bool vaildCheck(std::string FileRoot);
    std::vector<Server> parsing(std::string FileRoot);
};