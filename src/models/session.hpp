#ifndef SESSION_H
#define SESSION_H
#include <string>
#include <ctime>
struct Session_t {
    std::string token;
    std::string username;
    time_t created_at;
    time_t expires_at;
};
#endif