#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include "user.hpp"
#include "questions.hpp"
using namespace std;

class FileStore
{
public:
    bool saveUser_t(const User_t &u);
    optional<User_t> loadUser_t(const string &User_tname);
    bool User_tExists(const string &User_tname);

    string saveQuestion(const Question_t &q); // returns new ID
    optional<Question_t> loadQuestion(const string &id);
    vector<string> getQuestionsForUser_t(const string &User_tname);
    bool updateQuestion(const Question_t &q); // for answering
};
