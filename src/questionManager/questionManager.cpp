#include "questionManager.hpp"

std::string QuestionManager::askQuestion(const std::string &token,
                                         const std::string &to_user,
                                         const std::string &body,
                                         bool is_anonymous)
{
    auto username = this->auth.validateSession(token);
    if (!username.has_value())
    {
        std::cerr << "question manager: invalid session\n";
        return "";
    }
    if (to_user.empty())
    {
        std::cerr << "username is empty\n";
        return "";
    }
    if (body.empty())
    {
        std::cerr << "question body is empty\n";
        return "";
    }
    if (!this->store.userExists(to_user))
    {
        std::cerr << "question manager: user doesn't exist\n";
        return "";
    }
    Question_t question;
    question.from_user = username.value();
    question.to_user = to_user;
    question.body = body;
    question.is_anonymous = is_anonymous;
    question.created_at = time(nullptr);
    string question_id = this->store.saveQuestion(question);
    if (question_id.empty())
    {
        std::cerr << "question manager: failed to save question\n";
        return "";
    }

    return question_id;
}

std::string QuestionManager::askThreadQuestion(const std::string &token,
                                               const std::string &parent_id,
                                               const std::string &body,
                                               bool is_anonymous)
{
    auto username = this->auth.validateSession(token);
    if (!username.has_value())
    {
        std::cerr << "question manager: invalid session\n";
        return "";
    }
    if (body.empty())
    {
        std::cerr << "question body is empty\n";
        return "";
    }
    auto parent_question_opt = this->store.loadQuestion(parent_id);
    if (!parent_question_opt.has_value())
    {
        std::cerr << "question manager: parent question doesn't exist\n";
        return "";
    }
    if (!parent_question_opt->parent_id.empty())
    {
        std::cerr << "question manager: can't reply to a non top level question\n";
        return "";
    }

    Question_t question;
    question.from_user = username.value();
    question.to_user = parent_question_opt->to_user;
    question.body = body;
    question.is_anonymous = is_anonymous;
    question.parent_id = parent_id;
    question.created_at = time(nullptr);
    string question_id = this->store.saveQuestion(question);
    if (question_id.empty())
    {
        std::cerr << "question manager: failed to save question\n";
        return "";
    }

    parent_question_opt->children_ids.push_back(question_id);
    if (!this->store.updateQuestion(parent_question_opt.value()))
    {
        std::cerr << "question manager: failed to update parent question\n";
        return "";
    }
    return question_id;
}

bool QuestionManager::answerQuestion(const std::string &token,
                                     const std::string &question_id,
                                     const std::string &answer)
{
    auto username = this->auth.validateSession(token);
    if (!username.has_value())
    {
        std::cerr << "question manager: invalid session\n";
        return false;
    }
    if (answer.empty())
    {
        std::cerr << "answer is empty\n";
        return false;
    }
    auto question_opt = this->store.loadQuestion(question_id);
    if (!question_opt.has_value())
    {
        std::cerr << "question manager: question doesn't exist\n";
        return false;
    }
    if (username.value() != question_opt->to_user)
    {
        std::cerr << "question manager: user not authorized to answer the question\n";
        return false;
    }
    if (question_opt->is_deleted)
    {
        std::cerr << "question manager: can't answer a deleted question\n";
        return false;
    }

    if (!question_opt->answer.empty())
    {
        std::cerr << "question manager: question already answered\n";
        return false;
    }
    question_opt->answer = answer;
    question_opt->answered_at = time(nullptr);
    return this->store.updateQuestion(question_opt.value());
}

bool QuestionManager::deleteQuestion(const std::string &token,
                                     const std::string &question_id)
{
    auto username = this->auth.validateSession(token);

    if (username.has_value())
    {

        if (!question_id.empty())
        {
            optional<Question_t> question = this->store.loadQuestion(question_id);
            if (question.has_value())
            {

                if (question->is_deleted)
                {
                    std::cerr << "question manager: question already deleted\n";
                    return false;
                }
                if (question->to_user != username.value() && question->from_user != username.value())
                {
                    std::cerr << "question manager: user not authorized to delete the question\n";
                    return false;
                }
                else
                {
                    question->body = "[deleted]";
                    question->from_user = "[deleted]";
                    question->is_deleted = true;
                    return this->store.updateQuestion(question.value());
                }
            }
            else
            {
                std::cerr << "question manager: question doesn't exist\n";
                return false;
            }
        }
        else
        {
            std::cerr << "question manager: error empty question id\n";
            return false;
        }
    }
    else
    {
        std::cerr << "question manager:invalid session\n";
        return false;
    }
}

/*it should only return unanswered questions */
std::vector<Question_t> QuestionManager::getInbox(const std::string &token)
{
    auto username = this->auth.validateSession(token);
    if (!username.has_value())
    {
        std::cerr << "question manager: invalid session\n";
        return {};
    }

    std::vector<Question_t> inbox = this->store.getQuestionsForUser_t(username.value());
    for (auto it = inbox.begin(); it < inbox.end(); it++)
    {
        if (it->is_deleted || !it->answer.empty())
        {
            it = inbox.erase(it);
            it--;
        }
    }
    return inbox;
}