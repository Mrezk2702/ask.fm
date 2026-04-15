#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include "storage/FileStore.hpp"
#include "auth/auth.hpp"
#include "questionManager/questionManager.hpp"
#include "models/user.hpp"
#include "models/questions.hpp"

namespace fs = std::filesystem;

// ============================================================================
// Base Fixture — shared setup for all test suites
// ============================================================================

class BaseTest : public ::testing::Test
{
protected:
    FileStore*       store;
    AuthManager*     auth;
    QuestionManager* qm;
    const fs::path   test_dir = "test_data";

    std::string alice_token;
    std::string bob_token;

    void SetUp() override
    {
        fs::remove_all(test_dir);
        store = new FileStore(test_dir);
        auth  = new AuthManager(*store);
        qm    = new QuestionManager(*store, *auth);

        auth->signup("alice", "pass123", "alice@mail.com");
        auth->signup("bob",   "pass123", "bob@mail.com");
        alice_token = auth->login("alice", "pass123").value();
        bob_token   = auth->login("bob",   "pass123").value();
    }

    void TearDown() override
    {
        delete qm;
        delete auth;
        delete store;
        fs::remove_all(test_dir);
    }

    // ask a question from bob to alice
    std::string askBobToAlice(const std::string& body = "What is your favorite language?",
                               bool is_anonymous = false)
    {
        return qm->askQuestion(bob_token, "alice", body, is_anonymous);
    }

    // ask a question from alice to bob
    std::string askAliceToBob(const std::string& body = "What is your favorite language?",
                               bool is_anonymous = false)
    {
        return qm->askQuestion(alice_token, "bob", body, is_anonymous);
    }

    // ask and answer a question, returns question id
    std::string askAndAnswer(const std::string& body   = "What is your favorite language?",\
                              const std::string& answer = "C++ obviously!")
    {
        std::string id = askBobToAlice(body);
        qm->answerQuestion(alice_token, id, answer);
        return id;
    }
};

// ============================================================================
// FileStore — User Tests
// ============================================================================

class FileStoreUserTest : public BaseTest {};

TEST_F(FileStoreUserTest, SaveUser_ValidUser_ReturnsTrue)
{
    User_t u{};
    u.username = "charlie"; u.email = "c@mail.com";
    u.salt = "s"; u.password_hash = "h"; u.created_at = 0;
    EXPECT_TRUE(store->saveUser_t(u));
}

TEST_F(FileStoreUserTest, SaveUser_CreatesFile)
{
    User_t u{};
    u.username = "charlie"; u.email = "c@mail.com";
    u.salt = "s"; u.password_hash = "h"; u.created_at = 0;
    store->saveUser_t(u);
    EXPECT_TRUE(fs::exists(test_dir / "users" / "charlie.txt"));
}

TEST_F(FileStoreUserTest, LoadUser_ExistingUser_ReturnsValue)
{
    EXPECT_TRUE(store->loadUser_t("alice").has_value());
}

TEST_F(FileStoreUserTest, LoadUser_NonExistentUser_ReturnsNullopt)
{
    EXPECT_FALSE(store->loadUser_t("nobody").has_value());
}

TEST_F(FileStoreUserTest, LoadUser_PreservesUsername)
{
    EXPECT_EQ(store->loadUser_t("alice")->username, "alice");
}

TEST_F(FileStoreUserTest, LoadUser_PreservesEmail)
{
    EXPECT_EQ(store->loadUser_t("alice")->email, "alice@mail.com");
}

TEST_F(FileStoreUserTest, UserExists_ExistingUser_ReturnsTrue)
{
    EXPECT_TRUE(store->userExists("alice"));
}

TEST_F(FileStoreUserTest, UserExists_NonExistentUser_ReturnsFalse)
{
    EXPECT_FALSE(store->userExists("nobody"));
}

// ============================================================================
// FileStore — Session Tests
// ============================================================================

class FileStoreSessionTest : public BaseTest {};

TEST_F(FileStoreSessionTest, SaveSession_ReturnsTrue)
{
    EXPECT_TRUE(store->saveSession("tok1", "alice"));
}

TEST_F(FileStoreSessionTest, SaveSession_CreatesFile)
{
    store->saveSession("tok1", "alice");
    EXPECT_TRUE(fs::exists(test_dir / "sessions" / "sess_tok1.txt"));
}

TEST_F(FileStoreSessionTest, LoadSession_ExistingToken_ReturnsValue)
{
    store->saveSession("tok1", "alice");
    EXPECT_TRUE(store->loadSession("tok1").has_value());
}

TEST_F(FileStoreSessionTest, LoadSession_NonExistentToken_ReturnsNullopt)
{
    EXPECT_FALSE(store->loadSession("badtoken").has_value());
}

TEST_F(FileStoreSessionTest, LoadSession_PreservesUsername)
{
    store->saveSession("tok1", "alice");
    EXPECT_EQ(store->loadSession("tok1")->username, "alice");
}

TEST_F(FileStoreSessionTest, LoadSession_PreservesToken)
{
    store->saveSession("tok1", "alice");
    EXPECT_EQ(store->loadSession("tok1")->token, "tok1");
}

TEST_F(FileStoreSessionTest, LoadSession_ExpiresAtAfterCreatedAt)
{
    store->saveSession("tok1", "alice");
    auto s = store->loadSession("tok1");
    EXPECT_GT(s->expires_at, s->created_at);
}

TEST_F(FileStoreSessionTest, DeleteSession_ExistingToken_ReturnsTrue)
{
    store->saveSession("tok1", "alice");
    EXPECT_TRUE(store->deleteSession("tok1"));
}

TEST_F(FileStoreSessionTest, DeleteSession_RemovesFile)
{
    store->saveSession("tok1", "alice");
    store->deleteSession("tok1");
    EXPECT_FALSE(fs::exists(test_dir / "sessions" / "sess_tok1.txt"));
}

TEST_F(FileStoreSessionTest, DeleteSession_NonExistentToken_ReturnsFalse)
{
    EXPECT_FALSE(store->deleteSession("badtoken"));
}

// ============================================================================
// FileStore — Question Tests
// ============================================================================

class FileStoreQuestionTest : public BaseTest {};

TEST_F(FileStoreQuestionTest, SaveQuestion_ReturnsNonEmptyID)
{
    Question_t q{}; q.from_user="bob"; q.to_user="alice"; q.body="hi";
    EXPECT_FALSE(store->saveQuestion(q).empty());
}

TEST_F(FileStoreQuestionTest, SaveQuestion_CreatesFile)
{
    Question_t q{}; q.from_user="bob"; q.to_user="alice"; q.body="hi";
    std::string id = store->saveQuestion(q);
    EXPECT_TRUE(fs::exists(test_dir / "questions" / ("q_" + id + ".txt")));
}

TEST_F(FileStoreQuestionTest, SaveQuestion_IDsAreUnique)
{
    Question_t q{}; q.from_user="bob"; q.to_user="alice"; q.body="hi";
    EXPECT_NE(store->saveQuestion(q), store->saveQuestion(q));
}

TEST_F(FileStoreQuestionTest, LoadQuestion_ExistingID_ReturnsValue)
{
    Question_t q{}; q.from_user="bob"; q.to_user="alice"; q.body="hi";
    EXPECT_TRUE(store->loadQuestion(store->saveQuestion(q)).has_value());
}

TEST_F(FileStoreQuestionTest, LoadQuestion_NonExistentID_ReturnsNullopt)
{
    EXPECT_FALSE(store->loadQuestion("9999").has_value());
}

TEST_F(FileStoreQuestionTest, LoadQuestion_PreservesFromUser)
{
    Question_t q{}; q.from_user="bob"; q.to_user="alice"; q.body="hi";
    EXPECT_EQ(store->loadQuestion(store->saveQuestion(q))->from_user, "bob");
}

TEST_F(FileStoreQuestionTest, LoadQuestion_PreservesToUser)
{
    Question_t q{}; q.from_user="bob"; q.to_user="alice"; q.body="hi";
    EXPECT_EQ(store->loadQuestion(store->saveQuestion(q))->to_user, "alice");
}

TEST_F(FileStoreQuestionTest, LoadQuestion_PreservesBody)
{
    Question_t q{}; q.from_user="bob"; q.to_user="alice"; q.body="hello?";
    EXPECT_EQ(store->loadQuestion(store->saveQuestion(q))->body, "hello?");
}

TEST_F(FileStoreQuestionTest, LoadQuestion_PreservesIsAnonymous)
{
    Question_t q{}; q.from_user="bob"; q.to_user="alice"; q.body="hi"; q.is_anonymous=true;
    EXPECT_TRUE(store->loadQuestion(store->saveQuestion(q))->is_anonymous);
}

TEST_F(FileStoreQuestionTest, LoadQuestion_PreservesParentID)
{
    Question_t q{}; q.from_user="bob"; q.to_user="alice"; q.body="hi"; q.parent_id="0001";
    EXPECT_EQ(store->loadQuestion(store->saveQuestion(q))->parent_id, "0001");
}

TEST_F(FileStoreQuestionTest, LoadQuestion_PreservesChildren)
{
    Question_t q{}; q.from_user="bob"; q.to_user="alice"; q.body="hi";
    q.children_ids = {"0002", "0003"};
    EXPECT_EQ(store->loadQuestion(store->saveQuestion(q))->children_ids,
              (std::vector<std::string>{"0002", "0003"}));
}

TEST_F(FileStoreQuestionTest, UpdateQuestion_AnswerPersists)
{
    Question_t q{}; q.from_user="bob"; q.to_user="alice"; q.body="hi";
    auto id     = store->saveQuestion(q);
    auto loaded = store->loadQuestion(id);
    loaded->answer = "C++!";
    store->updateQuestion(loaded.value());
    EXPECT_EQ(store->loadQuestion(id)->answer, "C++!");
}

TEST_F(FileStoreQuestionTest, UpdateQuestion_EmptyID_ReturnsFalse)
{
    Question_t q{};
    EXPECT_FALSE(store->updateQuestion(q));
}

TEST_F(FileStoreQuestionTest, UpdateQuestion_NonExistentID_ReturnsFalse)
{
    Question_t q{}; q.id = "9999";
    EXPECT_FALSE(store->updateQuestion(q));
}

TEST_F(FileStoreQuestionTest, GetQuestionsForUser_EmptyUsername_ReturnsEmpty)
{
    EXPECT_TRUE(store->getQuestionsForUser_t("").empty());
}

TEST_F(FileStoreQuestionTest, GetQuestionsForUser_NoQuestions_ReturnsEmpty)
{
    EXPECT_TRUE(store->getQuestionsForUser_t("alice").empty());
}

TEST_F(FileStoreQuestionTest, GetQuestionsForUser_ReturnsCorrectCount)
{
    Question_t q{}; q.from_user="bob"; q.to_user="alice"; q.body="hi";
    store->saveQuestion(q);
    store->saveQuestion(q);
    EXPECT_EQ(store->getQuestionsForUser_t("alice").size(), 2);
}

TEST_F(FileStoreQuestionTest, AddQuestionToUserIndex_DuplicateID_ReturnsFalse)
{
    store->addQuestionToUserIndex("alice", "0001");
    EXPECT_FALSE(store->addQuestionToUserIndex("alice", "0001"));
}

// ============================================================================
// Auth Tests
// ============================================================================

class AuthTest : public BaseTest {};

TEST_F(AuthTest, Signup_ValidArgs_ReturnsTrue)
{
    EXPECT_TRUE(auth->signup("charlie", "pass123", "c@mail.com"));
}

TEST_F(AuthTest, Signup_EmptyUsername_ReturnsFalse)
{
    EXPECT_FALSE(auth->signup("", "pass123", "c@mail.com"));
}

TEST_F(AuthTest, Signup_EmptyPassword_ReturnsFalse)
{
    EXPECT_FALSE(auth->signup("charlie", "", "c@mail.com"));
}

TEST_F(AuthTest, Signup_EmptyEmail_ReturnsFalse)
{
    EXPECT_FALSE(auth->signup("charlie", "pass123", ""));
}

TEST_F(AuthTest, Signup_DuplicateUsername_ReturnsFalse)
{
    EXPECT_FALSE(auth->signup("alice", "pass123", "alice@mail.com"));
}

TEST_F(AuthTest, Login_CorrectCredentials_ReturnsToken)
{
    EXPECT_TRUE(auth->login("alice", "pass123").has_value());
}

TEST_F(AuthTest, Login_WrongPassword_ReturnsNullopt)
{
    EXPECT_FALSE(auth->login("alice", "wrongpass").has_value());
}

TEST_F(AuthTest, Login_NonExistentUser_ReturnsNullopt)
{
    EXPECT_FALSE(auth->login("nobody", "pass123").has_value());
}

TEST_F(AuthTest, Login_EmptyUsername_ReturnsNullopt)
{
    EXPECT_FALSE(auth->login("", "pass123").has_value());
}

TEST_F(AuthTest, Login_EmptyPassword_ReturnsNullopt)
{
    EXPECT_FALSE(auth->login("alice", "").has_value());
}

TEST_F(AuthTest, Login_GeneratesUniqueTokens)
{
    auto t1 = auth->login("alice", "pass123");
    auto t2 = auth->login("alice", "pass123");
    ASSERT_TRUE(t1.has_value());
    ASSERT_TRUE(t2.has_value());
    EXPECT_NE(t1.value(), t2.value());
}

TEST_F(AuthTest, ValidateSession_ValidToken_ReturnsUsername)
{
    auto result = auth->validateSession(alice_token);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "alice");
}

TEST_F(AuthTest, ValidateSession_InvalidToken_ReturnsNullopt)
{
    EXPECT_FALSE(auth->validateSession("badtoken").has_value());
}

TEST_F(AuthTest, ValidateSession_EmptyToken_ReturnsNullopt)
{
    EXPECT_FALSE(auth->validateSession("").has_value());
}

TEST_F(AuthTest, Logout_ValidToken_ReturnsTrue)
{
    EXPECT_TRUE(auth->logout(alice_token));
}

TEST_F(AuthTest, Logout_InvalidatesSession)
{
    auth->logout(alice_token);
    EXPECT_FALSE(auth->validateSession(alice_token).has_value());
}

TEST_F(AuthTest, Logout_EmptyToken_ReturnsFalse)
{
    EXPECT_FALSE(auth->logout(""));
}

TEST_F(AuthTest, Logout_InvalidToken_ReturnsFalse)
{
    EXPECT_FALSE(auth->logout("badtoken"));
}

// ============================================================================
// QuestionManager — askQuestion Tests
// ============================================================================

class AskQuestionTest : public BaseTest {};

TEST_F(AskQuestionTest, AskQuestion_ValidArgs_ReturnsID)
{
    EXPECT_FALSE(qm->askQuestion(bob_token, "alice", "hello?", false).empty());
}

TEST_F(AskQuestionTest, AskQuestion_InvalidToken_ReturnsEmpty)
{
    EXPECT_TRUE(qm->askQuestion("badtoken", "alice", "hello?", false).empty());
}

TEST_F(AskQuestionTest, AskQuestion_EmptyToUser_ReturnsEmpty)
{
    EXPECT_TRUE(qm->askQuestion(bob_token, "", "hello?", false).empty());
}

TEST_F(AskQuestionTest, AskQuestion_EmptyBody_ReturnsEmpty)
{
    EXPECT_TRUE(qm->askQuestion(bob_token, "alice", "", false).empty());
}

TEST_F(AskQuestionTest, AskQuestion_NonExistentRecipient_ReturnsEmpty)
{
    EXPECT_TRUE(qm->askQuestion(bob_token, "nobody", "hello?", false).empty());
}

TEST_F(AskQuestionTest, AskQuestion_SetsFromUser)
{
    std::string id = askBobToAlice();
    EXPECT_EQ(store->loadQuestion(id)->from_user, "bob");
}

TEST_F(AskQuestionTest, AskQuestion_SetsToUser)
{
    std::string id = askBobToAlice();
    EXPECT_EQ(store->loadQuestion(id)->to_user, "alice");
}

TEST_F(AskQuestionTest, AskQuestion_SetsIsAnonymous)
{
    std::string id = qm->askQuestion(bob_token, "alice", "hello?", true);
    EXPECT_TRUE(store->loadQuestion(id)->is_anonymous);
}

TEST_F(AskQuestionTest, AskQuestion_AppearsInRecipientIndex)
{
    askBobToAlice();
    EXPECT_FALSE(store->getQuestionsForUser_t("alice").empty());
}

// ============================================================================
// QuestionManager — askThreadQuestion Tests
// ============================================================================

class AskThreadQuestionTest : public BaseTest {};

TEST_F(AskThreadQuestionTest, AskThreadQuestion_ValidArgs_ReturnsID)
{
    std::string parent_id = askBobToAlice();
    EXPECT_FALSE(qm->askThreadQuestion(alice_token, parent_id, "why?", false).empty());
}

TEST_F(AskThreadQuestionTest, AskThreadQuestion_InvalidToken_ReturnsEmpty)
{
    std::string parent_id = askBobToAlice();
    EXPECT_TRUE(qm->askThreadQuestion("badtoken", parent_id, "why?", false).empty());
}

TEST_F(AskThreadQuestionTest, AskThreadQuestion_EmptyBody_ReturnsEmpty)
{
    std::string parent_id = askBobToAlice();
    EXPECT_TRUE(qm->askThreadQuestion(alice_token, parent_id, "", false).empty());
}

TEST_F(AskThreadQuestionTest, AskThreadQuestion_NonExistentParent_ReturnsEmpty)
{
    EXPECT_TRUE(qm->askThreadQuestion(alice_token, "9999", "why?", false).empty());
}

TEST_F(AskThreadQuestionTest, AskThreadQuestion_ReplyToReply_ReturnsEmpty)
{
    std::string parent_id = askBobToAlice();
    std::string reply_id  = qm->askThreadQuestion(alice_token, parent_id, "why?", false);
    EXPECT_TRUE(qm->askThreadQuestion(bob_token, reply_id, "because!", false).empty());
}

TEST_F(AskThreadQuestionTest, AskThreadQuestion_SetsParentID)
{
    std::string parent_id = askBobToAlice();
    std::string reply_id  = qm->askThreadQuestion(alice_token, parent_id, "why?", false);
    EXPECT_EQ(store->loadQuestion(reply_id)->parent_id, parent_id);
}

TEST_F(AskThreadQuestionTest, AskThreadQuestion_UpdatesParentChildren)
{
    std::string parent_id = askBobToAlice();
    std::string reply_id  = qm->askThreadQuestion(alice_token, parent_id, "why?", false);
    auto parent           = store->loadQuestion(parent_id);
    ASSERT_EQ(parent->children_ids.size(), 1);
    EXPECT_EQ(parent->children_ids[0], reply_id);
}

TEST_F(AskThreadQuestionTest, AskThreadQuestion_MultipleReplies_AllLinkedToParent)
{
    std::string parent_id = askBobToAlice();
    qm->askThreadQuestion(alice_token, parent_id, "reply1", false);
    qm->askThreadQuestion(bob_token,   parent_id, "reply2", false);
    EXPECT_EQ(store->loadQuestion(parent_id)->children_ids.size(), 2);
}

// ============================================================================
// QuestionManager — answerQuestion Tests
// ============================================================================

class AnswerQuestionTest : public BaseTest {};

TEST_F(AnswerQuestionTest, AnswerQuestion_ValidArgs_ReturnsTrue)
{
    EXPECT_TRUE(qm->answerQuestion(alice_token, askBobToAlice(), "C++!"));
}

TEST_F(AnswerQuestionTest, AnswerQuestion_InvalidToken_ReturnsFalse)
{
    EXPECT_FALSE(qm->answerQuestion("badtoken", askBobToAlice(), "C++!"));
}

TEST_F(AnswerQuestionTest, AnswerQuestion_EmptyAnswer_ReturnsFalse)
{
    EXPECT_FALSE(qm->answerQuestion(alice_token, askBobToAlice(), ""));
}

TEST_F(AnswerQuestionTest, AnswerQuestion_NonExistentQuestion_ReturnsFalse)
{
    EXPECT_FALSE(qm->answerQuestion(alice_token, "9999", "C++!"));
}

TEST_F(AnswerQuestionTest, AnswerQuestion_WrongUser_ReturnsFalse)
{
    EXPECT_FALSE(qm->answerQuestion(bob_token, askBobToAlice(), "C++!"));
}

TEST_F(AnswerQuestionTest, AnswerQuestion_AlreadyAnswered_ReturnsFalse)
{
    EXPECT_FALSE(qm->answerQuestion(alice_token, askAndAnswer(), "another"));
}

TEST_F(AnswerQuestionTest, AnswerQuestion_DeletedQuestion_ReturnsFalse)
{
    std::string id = askBobToAlice();
    qm->deleteQuestion(alice_token, id);
    EXPECT_FALSE(qm->answerQuestion(alice_token, id, "C++!"));
}

TEST_F(AnswerQuestionTest, AnswerQuestion_AnswerPersists)
{
    std::string id = askBobToAlice();
    qm->answerQuestion(alice_token, id, "C++!");
    EXPECT_EQ(store->loadQuestion(id)->answer, "C++!");
}

TEST_F(AnswerQuestionTest, AnswerQuestion_SetsAnsweredAt)
{
    std::string id = askBobToAlice();
    qm->answerQuestion(alice_token, id, "C++!");
    EXPECT_GT(store->loadQuestion(id)->answered_at, 0);
}

// ============================================================================
// QuestionManager — deleteQuestion Tests
// ============================================================================

class DeleteQuestionTest : public BaseTest {};

TEST_F(DeleteQuestionTest, DeleteQuestion_RecipientCanDelete_ReturnsTrue)
{
    EXPECT_TRUE(qm->deleteQuestion(alice_token, askBobToAlice()));
}

TEST_F(DeleteQuestionTest, DeleteQuestion_SenderCanDelete_ReturnsTrue)
{
    EXPECT_TRUE(qm->deleteQuestion(bob_token, askBobToAlice()));
}

TEST_F(DeleteQuestionTest, DeleteQuestion_InvalidToken_ReturnsFalse)
{
    EXPECT_FALSE(qm->deleteQuestion("badtoken", askBobToAlice()));
}

TEST_F(DeleteQuestionTest, DeleteQuestion_EmptyID_ReturnsFalse)
{
    EXPECT_FALSE(qm->deleteQuestion(alice_token, ""));
}

TEST_F(DeleteQuestionTest, DeleteQuestion_NonExistentQuestion_ReturnsFalse)
{
    EXPECT_FALSE(qm->deleteQuestion(alice_token, "9999"));
}

TEST_F(DeleteQuestionTest, DeleteQuestion_UnauthorizedUser_ReturnsFalse)
{
    auth->signup("charlie", "pass123", "c@mail.com");
    auto charlie_token = auth->login("charlie", "pass123").value();
    EXPECT_FALSE(qm->deleteQuestion(charlie_token, askBobToAlice()));
}

TEST_F(DeleteQuestionTest, DeleteQuestion_AlreadyDeleted_ReturnsFalse)
{
    std::string id = askBobToAlice();
    qm->deleteQuestion(alice_token, id);
    EXPECT_FALSE(qm->deleteQuestion(alice_token, id));
}

TEST_F(DeleteQuestionTest, DeleteQuestion_SetsIsDeleted)
{
    std::string id = askBobToAlice();
    qm->deleteQuestion(alice_token, id);
    EXPECT_TRUE(store->loadQuestion(id)->is_deleted);
}

TEST_F(DeleteQuestionTest, DeleteQuestion_SetsBodyToDeleted)
{
    std::string id = askBobToAlice();
    qm->deleteQuestion(alice_token, id);
    EXPECT_EQ(store->loadQuestion(id)->body, "[deleted]");
}

TEST_F(DeleteQuestionTest, DeleteQuestion_ClearsFromUser)
{
    std::string id = askBobToAlice();
    qm->deleteQuestion(alice_token, id);
    EXPECT_EQ(store->loadQuestion(id)->from_user, "[deleted]");
}

// ============================================================================
// QuestionManager — getInbox Tests
// ============================================================================

class GetInboxTest : public BaseTest {};

TEST_F(GetInboxTest, GetInbox_InvalidToken_ReturnsEmpty)
{
    EXPECT_TRUE(qm->getInbox("badtoken").empty());
}

TEST_F(GetInboxTest, GetInbox_NoQuestions_ReturnsEmpty)
{
    EXPECT_TRUE(qm->getInbox(alice_token).empty());
}

TEST_F(GetInboxTest, GetInbox_UnansweredQuestion_Included)
{
    askBobToAlice();
    EXPECT_EQ(qm->getInbox(alice_token).size(), 1);
}

TEST_F(GetInboxTest, GetInbox_AnsweredQuestion_Excluded)
{
    askAndAnswer();
    EXPECT_TRUE(qm->getInbox(alice_token).empty());
}

TEST_F(GetInboxTest, GetInbox_DeletedQuestion_Excluded)
{
    std::string id = askBobToAlice();
    qm->deleteQuestion(alice_token, id);
    EXPECT_TRUE(qm->getInbox(alice_token).empty());
}

TEST_F(GetInboxTest, GetInbox_MixedQuestions_ReturnsOnlyUnanswered)
{
    askBobToAlice("unanswered");
    askAndAnswer("answered", "yes");
    auto inbox = qm->getInbox(alice_token);
    ASSERT_EQ(inbox.size(), 1);
    EXPECT_EQ(inbox[0].body, "unanswered");
}

// ============================================================================
// QuestionManager — getFeed Tests
// ============================================================================

class GetFeedTest : public BaseTest {};

TEST_F(GetFeedTest, GetFeed_EmptyUsername_ReturnsEmpty)
{
    EXPECT_TRUE(qm->getFeed("").empty());
}

TEST_F(GetFeedTest, GetFeed_NoQuestions_ReturnsEmpty)
{
    EXPECT_TRUE(qm->getFeed("alice").empty());
}

TEST_F(GetFeedTest, GetFeed_AskedQuestion_Answered_Included)
{
    std::string id = askAliceToBob();
    qm->answerQuestion(bob_token, id, "C++ obviously!");
    EXPECT_EQ(qm->getFeed("alice").size(), 1);
}

TEST_F(GetFeedTest, GetFeed_ReceivedQuestion_Answered_Included)
{
    std::string id = askBobToAlice();
    qm->answerQuestion(alice_token, id, "C++ obviously!");
    EXPECT_EQ(qm->getFeed("alice").size(), 1);
}

TEST_F(GetFeedTest, GetFeed_UnansweredAskedQuestion_Excluded)
{
    askAliceToBob();
    EXPECT_TRUE(qm->getFeed("alice").empty());
}

TEST_F(GetFeedTest, GetFeed_UnansweredReceivedQuestion_Excluded)
{
    askBobToAlice();
    EXPECT_TRUE(qm->getFeed("alice").empty());
}

TEST_F(GetFeedTest, GetFeed_DeletedQuestion_Excluded)
{
    std::string id = askAliceToBob();
    qm->answerQuestion(bob_token, id, "C++ obviously!");
    qm->deleteQuestion(alice_token, id);
    EXPECT_TRUE(qm->getFeed("alice").empty());
}

TEST_F(GetFeedTest, GetFeed_AskedAndReceived_OnlyAnswered)
{
    // Asked unanswered
    askAliceToBob("asked_unanswered");
    // Asked answered
    std::string asked_id = askAliceToBob("asked_answered");
    qm->answerQuestion(bob_token, asked_id, "yes");
    
    // Received unanswered
    askBobToAlice("received_unanswered");
    // Received answered
    std::string received_id = askBobToAlice("received_answered");
    qm->answerQuestion(alice_token, received_id, "yes");
    
    auto feed = qm->getFeed("alice");
    ASSERT_EQ(feed.size(), 2);
    // Both answered questions should be in feed
    std::vector<std::string> bodies;
    for (const auto& q : feed) {
        bodies.push_back(q.body);
    }
    EXPECT_TRUE(std::find(bodies.begin(), bodies.end(), "asked_answered") != bodies.end());
    EXPECT_TRUE(std::find(bodies.begin(), bodies.end(), "received_answered") != bodies.end());
}

// ============================================================================
// QuestionManager — getThread Tests
// ============================================================================

class GetThreadTest : public BaseTest {};

TEST_F(GetThreadTest, GetThread_EmptyID_ReturnsNullopt)
{
    EXPECT_FALSE(qm->getThread("").has_value());
}

TEST_F(GetThreadTest, GetThread_NonExistentID_ReturnsNullopt)
{
    EXPECT_FALSE(qm->getThread("9999").has_value());
}

TEST_F(GetThreadTest, GetThread_ReplyID_ReturnsNullopt)
{
    std::string parent_id = askBobToAlice();
    std::string reply_id  = qm->askThreadQuestion(alice_token, parent_id, "why?", false);
    EXPECT_FALSE(qm->getThread(reply_id).has_value());
}

TEST_F(GetThreadTest, GetThread_ValidTopLevelID_ReturnsThread)
{
    EXPECT_TRUE(qm->getThread(askBobToAlice()).has_value());
}

TEST_F(GetThreadTest, GetThread_ParentBodyCorrect)
{
    std::string id = askBobToAlice("favorite language?");
    EXPECT_EQ(qm->getThread(id)->parent.body, "favorite language?");
}

TEST_F(GetThreadTest, GetThread_NoReplies_RepliesEmpty)
{
    EXPECT_TRUE(qm->getThread(askBobToAlice())->replies.empty());
}

TEST_F(GetThreadTest, GetThread_WithReplies_CorrectCount)
{
    std::string parent_id = askBobToAlice();
    qm->askThreadQuestion(alice_token, parent_id, "reply1", false);
    qm->askThreadQuestion(bob_token,   parent_id, "reply2", false);
    EXPECT_EQ(qm->getThread(parent_id)->replies.size(), 2);
}

TEST_F(GetThreadTest, GetThread_ReplyBodyCorrect)
{
    std::string parent_id = askBobToAlice();
    qm->askThreadQuestion(alice_token, parent_id, "why though?", false);
    EXPECT_EQ(qm->getThread(parent_id)->replies[0].body, "why though?");
}

// ============================================================================
// Entry Point
// ============================================================================

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}