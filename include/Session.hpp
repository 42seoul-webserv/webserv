//
// Created by 김민규 on 2022/12/06.
//

#ifndef WEBSERV_SESSION_HPP
#define WEBSERV_SESSION_HPP

#include <map>
#include <string>
#include <ctime>
#include <iostream>


namespace WS {
    class Time {
    public:
        int _sec;
        int _min;
        int _hour;
        int _day;
        int _month;
        int _year;

    public:
        Time()
        {
          time_t curTime = time(NULL);          // get current time info
          struct tm* pLocal = gmtime(&curTime); // convert to struct for easy use
          _sec = pLocal->tm_sec;
          _min = pLocal->tm_min;
          _hour = pLocal->tm_hour;
          _day = pLocal->tm_mday;
          _month = pLocal->tm_mon;
          _year = pLocal->tm_year;
        }

        Time(struct tm* pLocal)
        {
          _sec = pLocal->tm_sec;
          _min = pLocal->tm_min;
          _hour = pLocal->tm_hour;
          _day = pLocal->tm_mday;
          _month = pLocal->tm_mon;
          _year = pLocal->tm_year;
        }

        // 현재 시간을 기준으로 위 time정보가 과거인지 판단.
        bool isPast()
        {
          time_t curTime = time(NULL);          // get current time info
          struct tm* pLocal = gmtime(&curTime); // convert to struct for easy use

          if (_year != pLocal->tm_year)
            return (_year > pLocal->tm_year);
          else if (this->_month != pLocal->tm_mon)
            return (_month > pLocal->tm_mon);
          else if (this->_day != pLocal->tm_mday)
            return (_day > pLocal->tm_mday);
          else if (this->_hour != pLocal->tm_hour)
            return (_hour > pLocal->tm_hour);
          else if (this->_min != pLocal->tm_min)
            return (_min > pLocal->tm_min);
          else if (this->_sec != pLocal->tm_sec)
            return (_sec > pLocal->tm_sec);
          else
            return (false);
        }

        Time& getByHourOffset(int hour_offset)
        {
          Time resultTime;
          this->_hour += hour_offset;
          return (*this);
        }
    };
}

class Session {
private:
    // [id : expireDate]
    std::map<std::string, WS::Time> _storage;

public:
    ~Session()
    {
      _storage.clear();
    }

public:
    // travers map, and delete expired session.
    // ! FIX Needed
    void clearExpiredID()
    {
      if (_storage.empty())
      {
        std::cerr << "Storage size : " << _storage.size() << std::endl;
        return ;
      }
      std::map<std::string, WS::Time>::iterator itr = _storage.begin();
      while (itr != _storage.end())
      {
        if (itr->second.isPast()) // if expired Session ID
          this->_storage.erase(itr->first);
        itr++;
      }
    }

    // validate given id
    bool isValid_ID(const std::string& clientID)
    {
      // traverse through storage. if given id is inside storage, then return true;
      std::map<std::string, WS::Time>::iterator itr = _storage.begin();
      while (itr != _storage.end())
      {
        if (clientID == itr->first) // if _storage has clientID
          return true;
        itr++;
      }
      return false;
    }

    void add(const std::string& id, const WS::Time& expire_time)
    {
      _storage[id] = expire_time;
    }
};


#endif //WEBSERV_SESSION_HPP
