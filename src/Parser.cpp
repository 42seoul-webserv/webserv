#include "Parser.hpp"
#include "Server.hpp"
#include <cctype>
//CommonParser
bool CommonParser::IsNodeElemEmpty(ParserNode node)
{
    if (node.elem.size() == 0)
        return true;
    return false;
}

ParserNode* CommonParser::GetNextNode(ParserNode node)
{
    return (node.next);
}

ParserNode* ConfigParser::getNode(size_t server_index, std::string categoly)
{
    ParserNode *temp = NULL;

    temp = nodevector[server_index].next;
    while (temp->categoly != categoly)
            temp = temp->next;
    return temp;
}

std::vector<std::string> ConfigParser::GetNodeElem(size_t server_index, std::string categoly, std::string key)
{
    ParserNode *temp = NULL;
    std::vector<std::string> empty;
    std::map<std::string, std::vector<std::string> >::iterator it;

    temp = nodevector[server_index].next;
    while (temp->categoly != categoly)
            temp = temp->next;
    it = temp->elem.find(key);
    if (it == temp->elem.end())
    {
        empty.resize(1);
        return empty;
    }
    return it->second;
}

void CommonParser::displayAll()
{
    for (unsigned int i = 0; i < nodevector.size(); ++i)
    {
        ParserNode *temp = &nodevector[i];
        while (temp)
        {
            std::cout << temp->categoly << std::endl;
            for (std::map<std::string, std::vector<std::string> >::iterator it = temp->elem.begin(); \
                it != temp->elem.end(); ++it)
            {
                std::cout << "  "<<it->first << " : ";
                for (unsigned long i = 0; i < it->second.size(); ++i)
                {
                    std::cout << it->second[i] << " ";
                }
                std::cout << std::endl;
            }
            temp = temp->next;
        }
    }
}

//ConfigParser
bool ConfigParser::vaildCheck(std::string FileRoot)
{
    std::stack<char> checkStack;
    std::filebuf fb;
    char getChar;

    if (fb.open(FileRoot, std::ios::in) == NULL)
        return false;
    std::istream is(&fb);
    while (is)
    {
        getChar = static_cast<char>(is.get());
        if (getChar == '{')
            checkStack.push(getChar);
        else if (getChar == '}')
        {
            if (checkStack.top() == '{')
                checkStack.pop();
            else
                checkStack.push(getChar);
        }
    }
    fb.close();
    if (checkStack.empty())
        return (true);
    return false;
}
// private
void ConfigParser::getElem(ParserNode* temp, std::string line)
{
    std::string key;
    std::string buffer;
    std::vector<std::string> value;

    for (std::string::iterator it = line.begin(); it != line.end(); ++it)
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
        if (*it == ':' && key.size() == 0)
        {
            key = buffer;
            buffer.clear();
        }
        else
            buffer += *it;
    }
    temp->elem[key] = value;
}

ParserNode* ConfigParser::EnterNode(ParserNode* temp, std::string line)
{
    while (temp->next)
        temp = temp->next;
    temp->next = new ParserNode;
    temp->next->prev = temp;
    temp->next->next = NULL;
    temp = temp->next;
    for (std::string::iterator it = line.begin(); it != line.end(); ++it)
    {
        if ((isblank(*it) && *(it + 1) != '/') || *it == '{')
            continue;
        temp->categoly += *it;
    }
    return temp;
}

CommonParser::~CommonParser()
{
    ParserNode *origin;
    ParserNode *temp;

    for (unsigned int i = 0; i < nodevector.size(); ++i)
    {
        origin = nodevector[i].next;
        while (origin)
        {
            temp = origin->next;
            delete origin;
            origin = temp;
        }
    }
}

void ConfigParser::parsingOneNode(std::istream& is)
{
    std::string buffer;
    ParserNode rootnode;
    ParserNode *temp = &rootnode;

    rootnode.next = NULL;
    rootnode.prev = NULL;
    while (!getline(is, buffer).eof())
    {
        if (buffer.size() == 0)
            continue;
        if (*buffer.rbegin() == ';')
            getElem(temp, buffer);
        else if(*buffer.rbegin() == '{')
            temp = EnterNode(temp, buffer);
        else if(*buffer.rbegin() == '}')
        {
            if (temp == rootnode.next)
                break;
            temp = rootnode.next;
        }
        buffer.clear();
    }
    if (rootnode.next)
        nodevector.push_back(rootnode);
}

void ConfigParser::getAllowMethods(std::vector<MethodType>& _allowMethods, std::string categoly, unsigned int server_index)
{
    std::vector<std::string> methods;

    methods = GetNodeElem(server_index, categoly, "allow_methods");
    for (unsigned int i = 0; i < methods.size(); ++i)
    {
        switch (getMethodType(methods[i]))
        {
        case GET:
            _allowMethods.push_back(GET);
            break;
        case POST:
            _allowMethods.push_back(POST);
            break;
        case PUT:
            _allowMethods.push_back(PUT);
            break;
        case PATCH:
            _allowMethods.push_back(PATCH);
            break;
        case DELETE:
            _allowMethods.push_back(DELETE);
            break;
        default:
            _allowMethods.push_back(UNDEFINED);
            break;
        }
    }
}

void ConfigParser::getLocationAttr(Server& server, unsigned int server_index)
{
    size_t found;
    std::string temp_cate;
    std::vector<std::string> temp2;
    Location location;

    for (ParserNode* temp = nodevector[server_index].next; temp != NULL; temp = temp->next)
    {
        found = temp->categoly.find("location ");
        if (found != std::string::npos)
        {
            temp_cate = temp->categoly;
            temp_cate.erase(temp_cate.begin(), temp_cate.begin() + 9);
            location._location = temp_cate;
            location._index = *(GetNodeElem(server_index, temp->categoly, "index").begin());
            location._root = *(GetNodeElem(server_index, temp->categoly, "root").begin());
            getAllowMethods(location._allowMethods, temp->categoly, server_index);
            location.client_max_body_size = 10000;
            location._cgiInfo = GetNodeElem(server_index, temp->categoly, "cgi_info");
            server._locations.push_back(location);
            location._allowMethods.clear();
            location._cgiInfo.clear();           
        }
    }
}

void ConfigParser::displayServer(Server& server)
{
    std::cout << "server _allowMethods : ";
    for (unsigned int i = 0; i < server._allowMethods.size(); ++i)
        std::cout << server._allowMethods[i] << " ";
    std::cout << std::endl;
    std::cout << "error_page : " << std::endl;
        for (std::map<StatusCode, std::string>::iterator it = server._errorPage.begin(); it != server._errorPage.end(); ++it)
        {
            std::cout << it->first << " : ";
            std::cout << it->second << std::endl;
        }
    std::cout << std::endl;
    for (unsigned int i = 0; i < server._locations.size(); ++i)
    {
        std::cout << "_location : " << server._locations[i]._location << std::endl;
        std::cout << "_index : " << server._locations[i]._index << std::endl;
        std::cout << "_root : " << server._locations[i]._root << std::endl;
        std::cout << "maxsize : " << server._locations[i].client_max_body_size << std::endl;
        std::cout << "location allow methods: ";
        for (unsigned int k = 0; k < server._locations[i]._allowMethods.size(); ++k)
            std::cout << server._locations[i]._allowMethods[k] << " ";
        std::cout << std::endl;
        std::cout << "_cgiInfo : ";
        for (unsigned int k = 0; k < server._locations[i]._cgiInfo.size(); ++k)
            std::cout << server._locations[i]._cgiInfo[k] << " ";
        std::cout << std::endl;
    }
}

void ConfigParser::getErrorPage(std::map<StatusCode, std::string>& _errorPage, unsigned int server_index)
{
    ParserNode* error_page_node = getNode(server_index, "error_page");
    std::map<std::string, std::vector<std::string> >::iterator it;

    if (error_page_node)
    {
        for (it = error_page_node->elem.begin(); it != error_page_node->elem.end(); ++it)
            _errorPage[std::stod(it->first)] = *(it->second.begin());
    }
}
//begin empty일때
void ConfigParser::getServerAttr(Server& server, unsigned int server_index)
{
    std::string hostLine;

    hostLine = *(GetNodeElem(server_index, "server", "listen").begin());
    hostLine.erase(hostLine.begin(), hostLine.begin() + hostLine.find(':') + 1);//find exception
    inet_pton(AF_INET, "0.0.0.0", &server._socketAddr.sin_addr);
    server._socketAddr.sin_port = ntohs(static_cast<uint16_t>(std::stod(hostLine)));
    server._socketAddr.sin_family = AF_INET;
    server._index = *(GetNodeElem(server_index, "server", "index").begin());
    server._root = *(GetNodeElem(server_index, "server", "root").begin());
    getAllowMethods(server._allowMethods, "server", server_index);
    getLocationAttr(server, server_index);
    getErrorPage(server._errorPage, server_index);
    displayServer(server);
}

std::vector<Server> ConfigParser::parsing(std::string FileRoot)
{
    std::filebuf fb;
    std::vector<Server> _serverList;

    if (fb.open(FileRoot, std::ios::in) == NULL)
        throw (std::runtime_error("open fail\n"));
    std::istream is(&fb);
    while (is)
        parsingOneNode(is);
    fb.close();
    _serverList.resize(nodevector.size());
    for (unsigned int i = 0; i < nodevector.size(); ++i)
        getServerAttr(_serverList[i], i);
    return (_serverList);
}