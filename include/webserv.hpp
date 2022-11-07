#ifndef WEBSERV_HPP
 #define WEBSERV_HPP

#include "GenericObject.hpp"
#include <string>

ObjectContainer* parseConfigure(char* configFilePath);
ObjectContainer* parseObject(const std::string& fileData, size_t idx);
#endif