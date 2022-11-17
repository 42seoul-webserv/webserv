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

std::vector<std::string> ConfigParser::GetNodeElem(size_t server_index, std::string categoly, std::string key)
{
    ParserNode *temp = NULL;
    std::map<std::string, std::vector<std::string> >::iterator it;

    temp = nodevector[server_index].next;
    while (temp->categoly != categoly)
            temp = temp->next;
    it = temp->elem.find(key);
    return it->second;
}

std::vector<std::string> RequestParser::GetNodeElem(std::string categoly, std::string key)
{
    std::map<std::string, std::vector<std::string> >::iterator it;

    it = nodevector[0].elem.end();
    for (unsigned int i = 0; i < nodevector.size(); ++i)
    {
        if (nodevector[i].categoly == categoly)
        {
            it = nodevector[i].elem.find(key);
            break;
        }
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
        if (isblank(*it) || *it == '{')
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

//begin empty일때
void ConfigParser::getServerAttr(Server& server, unsigned int server_index)
{
    std::string hostLine;

    hostLine = *(GetNodeElem(server_index, "server", "listen").begin());
    hostLine.erase(hostLine.begin(), hostLine.begin() + hostLine.find(':') + 1);//find exception
    std::cerr << hostLine << std::endl;
    inet_pton(AF_INET, "0.0.0.0", &server._socketAddr.sin_addr);
    server._socketAddr.sin_port = ntohs(static_cast<uint16_t>(std::stod(hostLine)));
    server._socketAddr.sin_family = AF_INET;
    server._index = *(GetNodeElem(server_index, "server", "index").begin());
    server._root = *(GetNodeElem(server_index, "server", "root").begin());
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
//RequestParser

//private
void RequestParser::addNode(std::string catename)
{
    ParserNode temp;

    temp.next = NULL;
    temp.prev = NULL;
    temp.categoly = catename;
    nodevector.push_back(temp);
}
void RequestParser::getStartLine(std::string& line)
{
    std::string buffer;
    unsigned int k = 0;
    std::vector<std::string> value;
    std::string key[] = {"method", "path", "protocol"};
    
    addNode("startline");
    for(unsigned int i = 0; i < line.size(); ++i)
    {
        if (isblank(line[i]))
        {
            value.push_back(buffer);
            nodevector[0].elem[key[k++]] = value;
            buffer.clear();
            value.clear();
        }
        else
            buffer += line[i];
    }
    value.push_back(buffer);
    nodevector[0].elem[key[k]] = value;
    line.clear();  
}

void RequestParser::getheader(std::string &line)
{
    std::string key;
    std::string buffer;
    std::vector<std::string> value;

    for (std::string::iterator it = line.begin(); it != line.end(); ++it)
    {
        if (isblank(*it))
            continue;
        if (*it == ':')
        {
            key = buffer;
            it++;
            if (isblank(*it))
                it++;
            buffer.assign(it, line.end());
            value.push_back(buffer);
            break;
        }
        else
            buffer += *it;
    }
    nodevector[1].elem[key] = value;
}

void RequestParser::getBody(std::string& line)
{
    addNode("body");
    std::vector<std::string> value;

    value.push_back(line);
    nodevector[2].elem["body"] = value;
}
//public
void RequestParser::parsing(std::string FileRoot)
{
    std::filebuf fb;
    std::string line;

    if (fb.open(FileRoot, std::ios::in) == NULL)
        return;
    std::istream is(&fb);
    getline(is, line);
    getStartLine(line);
    addNode("header");
    while (!getline(is, line).eof() && line.size() != 0)
        getheader(line);
    getline(is, line);
    getBody(line);
    fb.close();
}