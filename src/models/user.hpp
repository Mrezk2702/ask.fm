#ifndef USER_H_
#define USER_H_
#include <iostream>
#include <string>
using namespace std;

typedef struct User_t
{
    string username;
    string password_hash;
    string salt;
    string email;
    string bio;
    bool allow_anonymous;
    time_t created_at;

} User_t;
#endif