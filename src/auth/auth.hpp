#ifndef AUTH_HPP
#define AUTH_HPP
#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <storage/FileStore.hpp>


class AuthManager {
public:
/* a reference to file store because
    this choice makes the file storage one instance accross the application*/
    FileStore & store;
    explicit AuthManager(FileStore & store):store(store){}
    bool signup(const std::string& username, const std::string& password, const std::string& email);
    std::optional<std::string> login(const std::string& username, const std::string& password); // returns token
    bool logout(const std::string& token);
    std::optional<std::string> validateSession(const std::string& token); // returns username
    private:
    std::string generateSalt(int length = 16);
};

#endif