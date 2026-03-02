#include <iostream>
#include <string>
using namespace std;
#include <vector>


typedef struct question
{
        string id;
    string from_user;    // empty if anonymous
    string to_user;
    bool is_anonymous;
    string body;
    string answer;       // empty = unanswered
    string parent_id;    // empty = top-level
    vector<string> children_ids;
    time_t created_at;
    time_t answered_at;
}Question_t;