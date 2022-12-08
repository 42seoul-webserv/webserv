#ifndef WEBSERV_SESSION_HPP
#define WEBSERV_SESSION_HPP

#include <map>
#include <string>
#include <ctime>
#include <iostream>

#define SESSION_VALID   (1)
#define SESSION_INVALID (0)
#define SESSION_UNSET   (-1)

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
        Time();
        Time(struct tm* pLocal);
        // 현재 시간을 기준으로 위 time정보가 과거인지 판단.
        bool isPast();
        Time& getByHourOffset(int hour_offset);
    };
}

class Session {
private:
    // [id : expireDate]
    std::map<std::string, WS::Time> _storage;

public:
    ~Session();
    // delete expired id inside the storage
    void clearExpiredID();
    // validate given id
    bool isValid_ID(const std::string& clientID);
    // add session_id to the storage
    void add(const std::string& id, const WS::Time& expire_time);
    // find given id in storage
    std::map<std::string, WS::Time>::iterator find(const std::string& id);
    // generate random string
    static std::string gen_random_string(int len);
   
};


#endif //WEBSERV_SESSION_HPP
