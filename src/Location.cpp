#include "Location.hpp"
#include <sys/stat.h> // for lstat()

static std::string getLocation(const std::string& url)
{
  size_t lastSlashPos = url.rfind('/');

  if (lastSlashPos == 0)
  {
    return (url);
  }
  else
  {
    return (url.substr(0, lastSlashPos));
  }
}

bool Location::isMatchedLocation(const std::string& url) const
{
  std::string urlLocation = getLocation(url);

  return (urlLocation == _location);
}

// check url is in location first...
std::string Location::convertURLToLocationPath(const std::string& url) const
{
  std::string result = _root;
  std::string filePath;

  if (url.size() >= _location.size())
    filePath = url.substr(this->_location.size(), std::string::npos);
  result = _root + filePath;
  struct stat sb;
  // if last url-chunk is directory, add location's _index to url. [ Ex. /YoupiBane => /YoupiBane/index.html ]
  if (stat(result.c_str(), &sb) != -1 && S_ISDIR(sb.st_mode))
  {
    if (this->_autoindex == false)
      result += ("/" + _index);
  }
  return (result);
}

bool Location::isRedirect() const
{
  if (this->_redirect.first < 300 || this->_redirect.first > 399)
  {
    return (false);
  }
  else if (this->_redirect.second.empty())
  {
    return (false);
  }
  else
  { // if valid redirect status_code
    return (true);
  }
}
