#include <iostream>
#include <filesystem>
#include "storage/FileStore.hpp"
#include "auth/auth.hpp"
#include "models/user.hpp"
#include "models/questions.hpp"

namespace fs = std::filesystem;

void printQuestion(const Question_t &q)
{
    std::cout << "  ID:        " << q.id          << "\n";
    std::cout << "  From:      " << q.from_user   << "\n";
    std::cout << "  To:        " << q.to_user     << "\n";
    std::cout << "  Body:      " << q.body        << "\n";
    std::cout << "  Answer:    " << (q.answer.empty() ? "[unanswered]" : q.answer) << "\n";
    std::cout << "  Anonymous: " << (q.is_anonymous ? "yes" : "no") << "\n";
    std::cout << "  Parent:    " << (q.parent_id.empty() ? "[none]" : q.parent_id) << "\n";
    std::cout << "-----------------------------\n";
}

void testUsers(FileStore &store)
{
    std::cout << "\n========== USER TESTS ==========\n";

    // save a user
    User_t u{};
    u.username        = "alice";
    u.email           = "alice@mail.com";
    u.bio             = "Hello I am alice";
    u.allow_anonymous = true;
    u.created_at      = time(nullptr);
    u.salt            = "testsalt";
    u.password_hash   = "fakehash";

    bool saved = store.saveUser_t(u);
    std::cout << "saveUser_t(alice):     " << (saved ? "PASS" : "FAIL") << "\n";

    // load the user back
    auto loaded = store.loadUser_t("alice");
    std::cout << "loadUser_t(alice):     " << (loaded.has_value() ? "PASS" : "FAIL") << "\n";
    if (loaded.has_value())
    {
        std::cout << "  username: " << loaded->username << "\n";
        std::cout << "  email:    " << loaded->email    << "\n";
        std::cout << "  bio:      " << loaded->bio      << "\n";
    }

    // check user exists
    std::cout << "userExists(alice):     " << (store.userExists("alice") ? "PASS" : "FAIL") << "\n";
    std::cout << "userExists(nobody):    " << (!store.userExists("nobody") ? "PASS" : "FAIL") << "\n";

    // load non-existent user
    auto missing = store.loadUser_t("nobody");
    std::cout << "loadUser_t(nobody):    " << (!missing.has_value() ? "PASS" : "FAIL") << "\n";
}

void testAuth(FileStore &store)
{
    std::cout << "\n========== AUTH TESTS ==========\n";
    AuthManager auth(store);

    // signup
    bool s1 = auth.signup("bob", "password123", "bob@mail.com");
    std::cout << "signup(bob):           " << (s1 ? "PASS" : "FAIL") << "\n";

    // duplicate signup
    bool s2 = auth.signup("bob", "password123", "bob@mail.com");
    std::cout << "signup(bob) duplicate: " << (!s2 ? "PASS" : "FAIL") << "\n";

    // signup empty fields
    bool s3 = auth.signup("", "password123", "x@mail.com");
    std::cout << "signup(empty user):    " << (!s3 ? "PASS" : "FAIL") << "\n";

    // login correct
    auto token = auth.login("bob", "password123");
    std::cout << "login(bob correct):    " << (token.has_value() ? "PASS" : "FAIL") << "\n";

    // login wrong password
    auto bad = auth.login("bob", "wrongpass");
    std::cout << "login(bob wrong pass): " << (!bad.has_value() ? "PASS" : "FAIL") << "\n";

    // validate session
    if (token.has_value())
    {
        auto username = auth.validateSession(token.value());
        std::cout << "validateSession:       " << (username.has_value() ? "PASS" : "FAIL") << "\n";
        if (username.has_value())
            std::cout << "  logged in as: " << username.value() << "\n";

        // logout
        bool loggedOut = auth.logout(token.value());
        std::cout << "logout:                " << (loggedOut ? "PASS" : "FAIL") << "\n";

        // validate after logout
        auto afterLogout = auth.validateSession(token.value());
        std::cout << "validate after logout: " << (!afterLogout.has_value() ? "PASS" : "FAIL") << "\n";
    }
}

void testQuestions(FileStore &store)
{
    std::cout << "\n========== QUESTION TESTS ==========\n";

    // save a question
    Question_t q{};
    q.from_user    = "bob";
    q.to_user      = "alice";
    q.body         = "What is your favorite language?";
    q.is_anonymous = false;
    q.created_at   = time(nullptr);

    string id = store.saveQuestion(q);
    std::cout << "saveQuestion:          " << (!id.empty() ? "PASS" : "FAIL") << "\n";
    std::cout << "  generated ID: " << id << "\n";

    // load it back
    auto loaded = store.loadQuestion(id);
    std::cout << "loadQuestion:          " << (loaded.has_value() ? "PASS" : "FAIL") << "\n";
    if (loaded.has_value())
        printQuestion(loaded.value());

    // update — answer the question
    if (loaded.has_value())
    {
        Question_t answered     = loaded.value();
        answered.answer         = "C++ obviously!";
        answered.answered_at    = time(nullptr);

        bool updated = store.updateQuestion(answered);
        std::cout << "updateQuestion:        " << (updated ? "PASS" : "FAIL") << "\n";

        // verify answer was saved
        auto reloaded = store.loadQuestion(id);
        if (reloaded.has_value())
        {
            std::cout << "answer persisted:      "
                      << (reloaded->answer == "C++ obviously!" ? "PASS" : "FAIL") << "\n";
        }
    }

    // save a thread reply
    Question_t reply{};
    reply.from_user    = "alice";
    reply.to_user      = "bob";
    reply.body         = "Why do you ask?";
    reply.parent_id    = id;
    reply.is_anonymous = false;
    reply.created_at   = time(nullptr);

    string reply_id = store.saveQuestion(reply);
    std::cout << "saveQuestion(thread):  " << (!reply_id.empty() ? "PASS" : "FAIL") << "\n";

    // get questions for user
    auto questions = store.getQuestionsForUser_t("alice");
    std::cout << "getQuestionsForUser:   " << (!questions.empty() ? "PASS" : "FAIL") << "\n";
    std::cout << "  question count: " << questions.size() << "\n";
    for (const auto &q : questions)
        printQuestion(q);
}

int main()
{
    // clean slate — remove old test data
    fs::remove_all("data");

    FileStore store(fs::path("data"));

    testUsers(store);
    testAuth(store);
    testQuestions(store);

    std::cout << "\n========== ALL TESTS DONE ==========\n";
    return 0;
}