#ifndef QUESTION_MANGAER_H
#define QUESTION_MANAGER_H
#include <string>
#include <auth/auth.hpp>
#include <storage/FileStore.hpp>
#include <vector>
#include <optional>
#include "models/questions.hpp"
#include "models/user.hpp"
#include "models/session.hpp"

class QuestionManager
{
public:
    explicit QuestionManager(FileStore &store, AuthManager &auth)
        : store(store), auth(auth) {}

    std::string askQuestion(const std::string &token,
                            const std::string &to_user,
                            const std::string &body,
                            bool is_anonymous);

    std::string askThreadQuestion(const std::string &token,
                                  const std::string &parent_id,
                                  const std::string &body,
                                  bool is_anonymous);

    bool answerQuestion(const std::string &token,
                        const std::string &question_id,
                        const std::string &answer);

    bool deleteQuestion(const std::string &token,
                        const std::string &question_id);

    std::vector<Question_t> getInbox(const std::string &token);

    std::vector<Question_t> getFeed(const std::string &username);

    std::optional<Question_t> getThread(const std::string &id);

private:
    FileStore   &store;
    AuthManager &auth;
};

#endif