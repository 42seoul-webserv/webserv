#include <iostream>
#include <fstream>
#include <sstream>

std::string getDataFromFile(char *filePath)
{
  std::ifstream infileStream;
  std::ostringstream ss;

  infileStream.open(filePath);
  assert(infileStream.bad() == 0);
  ss << infileStream.rdbuf();
  return (ss.str());
}

void  parseConfigure(char *configFilePath)
{
  const std::string fileData = getDataFromFile(configFilePath);

  std::cout << fileData;
}
