#include <iostream>
#include <string>
using namespace std;

typedef struct user
{
    string username;
    string password_hash;
    string email;
    string bio;
    bool allow_anonymous;
    time_t created_at;

} User_t;