
#include "webserv.hpp"
#include "GenericObject.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

std::string getDataFromFile(const char *filePath)
{
  std::ifstream infileStream;
  std::ostringstream ss;
  std::string fileString;

  infileStream.open(filePath);
  assert(infileStream.bad() == 0);
  ss << infileStream.rdbuf();
  fileString = ss.str();
  return (fileString);
}

std::string getKey(const std::string& str, size_t& idx)
{
  std::string key;
  size_t startIdx = idx;

  while (str[idx] && !isspace(str[idx]) && str[idx] != ':')
    ++idx;
  key = str.substr(startIdx, idx - startIdx);
  return (key);
}

ft::BaseObject parseConfigure(const char* configFilePath)
{
  const std::string fileData = getDataFromFile(configFilePath);
  const ft::BaseObject configObject = parseObject(fileData, 0);
}
