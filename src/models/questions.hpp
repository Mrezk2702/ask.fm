#ifndef QUESTIONS_H
#define QUESTIONS_H
#include <iostream>
#include <string>
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
    bool is_deleted;
}Question_t;

typedef struct Thread_t {
    Question_t parent;
    std::vector<Question_t> replies;
}Thread_t;

#endif