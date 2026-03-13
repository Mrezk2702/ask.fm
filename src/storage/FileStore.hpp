#ifndef FILE_STORE_H_
#define FILE_STORE_H_
#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include "models/user.hpp"
#include "models/questions.hpp"
#include <filesystem>
#include "id_generator.hpp"
#include <ctime>
using namespace std;
namespace fs = std::filesystem;
class FileStore
{
private:
    string data_dir = "data";
    string user_dir = "users";
    string question_dir = "questions";
    string sessions_dir = "sessions";
    IDGenerator id_gen;
    const time_t day_in_seconds=86400;

public:
    FileStore(const fs::path &data_dir);
    bool saveUser_t(const User_t &u);
    optional<User_t> loadUser_t(const string &User_tname);
    bool userExists(const string &User_tname);

    string saveQuestion(const Question_t &q); // returns new ID
    optional<Question_t> loadQuestion(const string &id);
    vector<string> getQuestionsForUser_t(const string &User_tname);
    bool updateQuestion(const Question_t &q); // for answering
    bool saveSession(const string& token,const string& username);
};

#endif
