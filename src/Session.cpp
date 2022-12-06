#include <unistd.h>
#include "Session.hpp"

WS::Time::Time()
{
  time_t curTime = time(NULL);          // get current time info
  struct tm *pLocal = gmtime(&curTime); // convert to struct for easy use
  _sec = pLocal->tm_sec;
  _min = pLocal->tm_min;
  _hour = pLocal->tm_hour;
  _day = pLocal->tm_mday;
  _month = pLocal->tm_mon;
  _year = pLocal->tm_year;
}

WS::Time::Time(struct tm *pLocal)
{
  _sec = pLocal->tm_sec;
  _min = pLocal->tm_min;
  _hour = pLocal->tm_hour;
  _day = pLocal->tm_mday;
  _month = pLocal->tm_mon;
  _year = pLocal->tm_year;
}

// 현재 시간을 기준으로 위 time정보가 과거인지 판단.
bool WS::Time::isPast()
{
  time_t curTime = time(NULL);          // get current time info
  struct tm *pLocal = gmtime(&curTime); // convert to struct for easy use

  if (_year != pLocal->tm_year)
    return (_year < pLocal->tm_year);
  else if (this->_month != pLocal->tm_mon)
    return (_month < pLocal->tm_mon);
  else if (this->_day != pLocal->tm_mday)
    return (_day < pLocal->tm_mday);
  else if (this->_hour != pLocal->tm_hour)
    return (_hour < pLocal->tm_hour);
  else if (this->_min != pLocal->tm_min)
    return (_min < pLocal->tm_min);
  else if (this->_sec != pLocal->tm_sec)
    return (_sec < pLocal->tm_sec);
  else
    return (true);
}

WS::Time &WS::Time::getByHourOffset(int hour_offset)
{
  Time resultTime;
  this->_hour += hour_offset;
  return (*this);
}

Session::~Session()
{
  _storage.clear();
}

// travers map, and delete expired session.
// ! FIX Needed
void Session::clearExpiredID()
{
  if (_storage.empty())
    return;

  std::map<std::string, WS::Time>::iterator itr = _storage.begin();
  while (itr != _storage.end())
  {
    if (itr->second.isPast()) // if expired Session ID
      this->_storage.erase((itr++)->first);
    else
      ++itr;
  }
}

// validate given id
bool Session::isValid_ID(const std::string &clientID)
{
  if (Session::find(clientID) != _storage.end())
    return (true);
  return (false);
}

void Session::add(const std::string &id, const WS::Time &expire_time)
{
  _storage[id] = expire_time;
}

std::map<std::string, WS::Time>::iterator Session::find(const std::string &id)
{
  return _storage.find(id);
}

std::string Session::gen_random_string(int len)
{
  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  std::string tmp_s;
  srand((unsigned)time(NULL) * getpid());
  tmp_s.reserve(len);

  for (int i = 0; i < len; ++i)
  {
    tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
  }
  return tmp_s;
}
