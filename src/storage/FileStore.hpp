#ifndef FILE_STORE_H_
#define FILE_STORE_H_
#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include "models/user.hpp"
#include "models/questions.hpp"
using namespace std;

class FileStore
{
    private:
    string data_dir="data";
    string user_dir="users";
    string sessions_dir="sessions";

public:
    bool saveUser_t(const User_t &u);
    optional<User_t> loadUser_t(const string &User_tname);
    bool User_tExists(const string &User_tname);

    string saveQuestion(const Question_t &q); // returns new ID
    optional<Question_t> loadQuestion(const string &id);
    vector<string> getQuestionsForUser_t(const string &User_tname);
    bool updateQuestion(const Question_t &q); // for answering
};

#endif
