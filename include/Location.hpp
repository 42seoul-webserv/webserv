#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "WebservDefines.hpp"

class Location
{
public:
    std::string _location;
    std::string _index;              // ex. _index.html
    std::string _root;               // ex ./myDir/...
    std::vector<MethodType> allowMethods;       // ex. GET POST DELETE ...
    int clientMaxBodySize;  // (--> max size of client body request)   --> defaults to 8000 bytes
    std::vector<std::string> cgiInfo;      // ex. name: cgi_tester, arg: hello_world
    std::pair<StatusCode, std::string> _redirect;   // ex. 301 https://profile.intra.42.fr/
    bool _autoindex; // autoindex flag (on | off)

public:
    bool isMatchedLocation(const std::string& url) const;

    // check if server has valid redirection setting.
    bool isRedirect() const;

    std::string convertURLToLocationPath(const std::string& url) const;
};

#endif //LOCATION_HPP
