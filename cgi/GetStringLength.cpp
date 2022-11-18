#include <iostream>
#include <string>
#include <unistd.h>

#define BUFFER_SIZE 10240

static size_t getByte(unsigned char data)
{
  const unsigned char BIT_MASK[] = {0, 0x0, 0xC0, 0xE0, 0xF0};

  for (size_t byte = 4; byte > 1; --byte)
  {
    if ((data & BIT_MASK[byte]) == BIT_MASK[byte])
      return (byte);
  }
  return (data >> 7 == 0);
}

size_t getWordLength(const std::string& str)
{
  size_t utf8_char_count = 0;
  const size_t SIZE = str.length();
  int byte;

  for (size_t idx = 0; idx < SIZE; )
  {
    if ((byte = getByte(str[idx])) == 0)
      throw (std::runtime_error("invalid byte format"));
    idx += byte;
    ++utf8_char_count;
  }
  return (utf8_char_count);
}

// CGI ENVIRON..
// FIXME : envp 처리가 필요함.
int main(int argc, char *argv[], char *envp[]) // content-length
{
  char buffer[BUFFER_SIZE] = {0};
  std::string input;

  (void) argv;
  (void) argc;
  (void) envp;

  int readSize = 0;
  while ((readSize = read(0, buffer, BUFFER_SIZE)) > 0)
  {
    input += buffer;
    memset(buffer, 0, sizeof(buffer));
  }
  if (readSize < 0)
    std::cout << "Read Failed\n";
  else
  {
    try
    {
      size_t stringLength = getWordLength(input);

      std::cout << "--Input--\n";
      std::cout << input << "\n";
      std::cout << "Word Length is : " << stringLength << "\n";
      std::cout << "Byte Length is : "  << input.length() << "\n";
    }
    catch (std::exception& e)
    {
      std::cout << "exception catched...\n";
    }
  }
  return (0);
}