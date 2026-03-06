#include <iostream>
#include <vector>
#include <string>
#include <optional>
using namespace std;


class AuthManager {
public:
    bool signup(const string& username, const string& password, const string& email);
    optional<string> login(const string& username, const string& password); // returns token
    bool logout(const string& token);
    optional<string> validateSession(const string& token); // returns username
};