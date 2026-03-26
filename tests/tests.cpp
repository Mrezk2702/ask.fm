#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "storage/FileStore.hpp"
#include "auth/auth.hpp"
#include "models/user.hpp"
#include "models/questions.hpp"

namespace fs = std::filesystem;

// ============================================================================
// Test Fixtures
// ============================================================================

class FileStoreTest : public ::testing::Test
{
protected:
    FileStore* store;
    const fs::path test_dir = "test_data";

    void SetUp() override
    {
        fs::remove_all(test_dir);
        store = new FileStore(test_dir);
    }

    void TearDown() override
    {
        delete store;
        fs::remove_all(test_dir);
    }

    // Helper — creates and saves a basic user
    User_t makeUser(const std::string& username = "alice")
    {
        User_t u{};
        u.username        = username;
        u.email           = username + "@mail.com";
        u.bio             = "bio of " + username;
        u.allow_anonymous = true;
        u.created_at      = 1710000000;
        u.salt            = "testsalt";
        u.password_hash   = "fakehash";
        return u;
    }

    // Helper — creates and saves a basic question
    Question_t makeQuestion(const std::string& from = "bob",
                            const std::string& to   = "alice")
    {
        Question_t q{};
        q.from_user    = from;
        q.to_user      = to;
        q.body         = "What is your favorite language?";
        q.is_anonymous = false;
        q.created_at   = 1710000000;
        q.answered_at  = 0;
        return q;
    }
};

class AuthTest : public ::testing::Test
{
protected:
    FileStore* store;
    AuthManager* auth;
    const fs::path test_dir = "test_auth_data";

    void SetUp() override
    {
        fs::remove_all(test_dir);
        store = new FileStore(test_dir);
        auth  = new AuthManager(*store);
    }

    void TearDown() override
    {
        delete auth;
        delete store;
        fs::remove_all(test_dir);
    }
};

// ============================================================================
// User Tests
// ============================================================================

TEST_F(FileStoreTest, SaveUser_ValidUser_ReturnsTrue)
{
    User_t u = makeUser("alice");
    EXPECT_TRUE(store->saveUser_t(u));
}

TEST_F(FileStoreTest, SaveUser_CreatesFile)
{
    User_t u = makeUser("alice");
    store->saveUser_t(u);
    EXPECT_TRUE(fs::exists(test_dir / "users" / "alice.txt"));
}

TEST_F(FileStoreTest, LoadUser_ExistingUser_ReturnsUser)
{
    store->saveUser_t(makeUser("alice"));
    auto result = store->loadUser_t("alice");
    EXPECT_TRUE(result.has_value());
}

TEST_F(FileStoreTest, LoadUser_NonExistentUser_ReturnsNullopt)
{
    auto result = store->loadUser_t("nobody");
    EXPECT_FALSE(result.has_value());
}

TEST_F(FileStoreTest, LoadUser_PreservesUsername)
{
    store->saveUser_t(makeUser("alice"));
    auto result = store->loadUser_t("alice");
    EXPECT_EQ(result->username, "alice");
}

TEST_F(FileStoreTest, LoadUser_PreservesEmail)
{
    store->saveUser_t(makeUser("alice"));
    auto result = store->loadUser_t("alice");
    EXPECT_EQ(result->email, "alice@mail.com");
}

TEST_F(FileStoreTest, LoadUser_PreservesBio)
{
    store->saveUser_t(makeUser("alice"));
    auto result = store->loadUser_t("alice");
    EXPECT_EQ(result->bio, "bio of alice");
}

TEST_F(FileStoreTest, LoadUser_PreservesAllowAnonymous)
{
    User_t u      = makeUser("alice");
    u.allow_anonymous = false;
    store->saveUser_t(u);
    auto result = store->loadUser_t("alice");
    EXPECT_FALSE(result->allow_anonymous);
}

TEST_F(FileStoreTest, LoadUser_PreservesCreatedAt)
{
    store->saveUser_t(makeUser("alice"));
    auto result = store->loadUser_t("alice");
    EXPECT_EQ(result->created_at, 1710000000);
}

TEST_F(FileStoreTest, UserExists_ExistingUser_ReturnsTrue)
{
    store->saveUser_t(makeUser("alice"));
    EXPECT_TRUE(store->userExists("alice"));
}

TEST_F(FileStoreTest, UserExists_NonExistentUser_ReturnsFalse)
{
    EXPECT_FALSE(store->userExists("nobody"));
}

// ============================================================================
// Session Tests
// ============================================================================

TEST_F(FileStoreTest, SaveSession_ValidArgs_ReturnsTrue)
{
    EXPECT_TRUE(store->saveSession("token123", "alice"));
}

TEST_F(FileStoreTest, SaveSession_CreatesFile)
{
    store->saveSession("token123", "alice");
    EXPECT_TRUE(fs::exists(test_dir / "sessions" / "sess_token123.txt"));
}

TEST_F(FileStoreTest, LoadSession_ExistingToken_ReturnsSession)
{
    store->saveSession("token123", "alice");
    auto result = store->loadSession("token123");
    EXPECT_TRUE(result.has_value());
}

TEST_F(FileStoreTest, LoadSession_NonExistentToken_ReturnsNullopt)
{
    auto result = store->loadSession("badtoken");
    EXPECT_FALSE(result.has_value());
}

TEST_F(FileStoreTest, LoadSession_PreservesUsername)
{
    store->saveSession("token123", "alice");
    auto result = store->loadSession("token123");
    EXPECT_EQ(result->username, "alice");
}

TEST_F(FileStoreTest, LoadSession_PreservesToken)
{
    store->saveSession("token123", "alice");
    auto result = store->loadSession("token123");
    EXPECT_EQ(result->token, "token123");
}

TEST_F(FileStoreTest, LoadSession_ExpiresAtAfterCreatedAt)
{
    store->saveSession("token123", "alice");
    auto result = store->loadSession("token123");
    EXPECT_GT(result->expires_at, result->created_at);
}

TEST_F(FileStoreTest, DeleteSession_ExistingToken_ReturnsTrue)
{
    store->saveSession("token123", "alice");
    EXPECT_TRUE(store->deleteSession("token123"));
}

TEST_F(FileStoreTest, DeleteSession_RemovesFile)
{
    store->saveSession("token123", "alice");
    store->deleteSession("token123");
    EXPECT_FALSE(fs::exists(test_dir / "sessions" / "sess_token123.txt"));
}

TEST_F(FileStoreTest, DeleteSession_NonExistentToken_ReturnsFalse)
{
    EXPECT_FALSE(store->deleteSession("badtoken"));
}

// ============================================================================
// Question Tests
// ============================================================================

TEST_F(FileStoreTest, SaveQuestion_ValidQuestion_ReturnsNonEmptyID)
{
    std::string id = store->saveQuestion(makeQuestion());
    EXPECT_FALSE(id.empty());
}

TEST_F(FileStoreTest, SaveQuestion_CreatesFile)
{
    std::string id = store->saveQuestion(makeQuestion());
    EXPECT_TRUE(fs::exists(test_dir / "questions" / ("q_" + id + ".txt")));
}

TEST_F(FileStoreTest, SaveQuestion_IDsAreUnique)
{
    std::string id1 = store->saveQuestion(makeQuestion());
    std::string id2 = store->saveQuestion(makeQuestion());
    EXPECT_NE(id1, id2);
}

TEST_F(FileStoreTest, LoadQuestion_ExistingID_ReturnsQuestion)
{
    std::string id = store->saveQuestion(makeQuestion());
    auto result    = store->loadQuestion(id);
    EXPECT_TRUE(result.has_value());
}

TEST_F(FileStoreTest, LoadQuestion_NonExistentID_ReturnsNullopt)
{
    auto result = store->loadQuestion("9999");
    EXPECT_FALSE(result.has_value());
}

TEST_F(FileStoreTest, LoadQuestion_PreservesFromUser)
{
    std::string id = store->saveQuestion(makeQuestion("bob", "alice"));
    auto result    = store->loadQuestion(id);
    EXPECT_EQ(result->from_user, "bob");
}

TEST_F(FileStoreTest, LoadQuestion_PreservesToUser)
{
    std::string id = store->saveQuestion(makeQuestion("bob", "alice"));
    auto result    = store->loadQuestion(id);
    EXPECT_EQ(result->to_user, "alice");
}

TEST_F(FileStoreTest, LoadQuestion_PreservesBody)
{
    std::string id = store->saveQuestion(makeQuestion());
    auto result    = store->loadQuestion(id);
    EXPECT_EQ(result->body, "What is your favorite language?");
}

TEST_F(FileStoreTest, LoadQuestion_PreservesIsAnonymous)
{
    Question_t q   = makeQuestion();
    q.is_anonymous = true;
    std::string id = store->saveQuestion(q);
    auto result    = store->loadQuestion(id);
    EXPECT_TRUE(result->is_anonymous);
}

TEST_F(FileStoreTest, LoadQuestion_PreservesParentID)
{
    Question_t q  = makeQuestion();
    q.parent_id   = "0001";
    std::string id = store->saveQuestion(q);
    auto result    = store->loadQuestion(id);
    EXPECT_EQ(result->parent_id, "0001");
}

TEST_F(FileStoreTest, LoadQuestion_PreservesChildren)
{
    Question_t q       = makeQuestion();
    q.children_ids     = {"0002", "0003"};
    std::string id     = store->saveQuestion(q);
    auto result        = store->loadQuestion(id);
    EXPECT_EQ(result->children_ids, (std::vector<std::string>{"0002", "0003"}));
}

TEST_F(FileStoreTest, UpdateQuestion_AnswerPersists)
{
    std::string id  = store->saveQuestion(makeQuestion());
    auto loaded     = store->loadQuestion(id);
    loaded->answer  = "C++ obviously!";
    store->updateQuestion(loaded.value());

    auto reloaded   = store->loadQuestion(id);
    EXPECT_EQ(reloaded->answer, "C++ obviously!");
}

TEST_F(FileStoreTest, UpdateQuestion_EmptyID_ReturnsFalse)
{
    Question_t q{};
    EXPECT_FALSE(store->updateQuestion(q));
}

TEST_F(FileStoreTest, UpdateQuestion_NonExistentID_ReturnsFalse)
{
    Question_t q{};
    q.id = "9999";
    EXPECT_FALSE(store->updateQuestion(q));
}

// ============================================================================
// User Question Index Tests
// ============================================================================

TEST_F(FileStoreTest, GetQuestionsForUser_NoQuestions_ReturnsEmpty)
{
    store->saveUser_t(makeUser("alice"));
    auto result = store->getQuestionsForUser_t("alice");
    EXPECT_TRUE(result.empty());
}

TEST_F(FileStoreTest, GetQuestionsForUser_EmptyUsername_ReturnsEmpty)
{
    auto result = store->getQuestionsForUser_t("");
    EXPECT_TRUE(result.empty());
}

TEST_F(FileStoreTest, GetQuestionsForUser_ReturnsCorrectCount)
{
    store->saveQuestion(makeQuestion("bob",     "alice"));
    store->saveQuestion(makeQuestion("charlie", "alice"));
    auto result = store->getQuestionsForUser_t("alice");
    EXPECT_EQ(result.size(), 2);
}

TEST_F(FileStoreTest, GetQuestionsForUser_QuestionsAddressedToUser)
{
    store->saveQuestion(makeQuestion("bob", "alice"));
    auto result = store->getQuestionsForUser_t("alice");
    for (const auto &q : result)
        EXPECT_EQ(q.to_user, "alice");
}

TEST_F(FileStoreTest, AddQuestionToUserIndex_DuplicateID_ReturnsFalse)
{
    store->saveUser_t(makeUser("alice"));
    store->addQuestionToUserIndex("alice", "0001");
    EXPECT_FALSE(store->addQuestionToUserIndex("alice", "0001"));
}

// ============================================================================
// Auth Tests
// ============================================================================

TEST_F(AuthTest, Signup_ValidArgs_ReturnsTrue)
{
    EXPECT_TRUE(auth->signup("alice", "password123", "alice@mail.com"));
}

TEST_F(AuthTest, Signup_EmptyUsername_ReturnsFalse)
{
    EXPECT_FALSE(auth->signup("", "password123", "alice@mail.com"));
}

TEST_F(AuthTest, Signup_EmptyPassword_ReturnsFalse)
{
    EXPECT_FALSE(auth->signup("alice", "", "alice@mail.com"));
}

TEST_F(AuthTest, Signup_EmptyEmail_ReturnsFalse)
{
    EXPECT_FALSE(auth->signup("alice", "password123", ""));
}

TEST_F(AuthTest, Signup_DuplicateUsername_ReturnsFalse)
{
    auth->signup("alice", "password123", "alice@mail.com");
    EXPECT_FALSE(auth->signup("alice", "password123", "alice@mail.com"));
}

TEST_F(AuthTest, Login_CorrectCredentials_ReturnsToken)
{
    auth->signup("alice", "password123", "alice@mail.com");
    auto token = auth->login("alice", "password123");
    EXPECT_TRUE(token.has_value());
}

TEST_F(AuthTest, Login_WrongPassword_ReturnsNullopt)
{
    auth->signup("alice", "password123", "alice@mail.com");
    auto token = auth->login("alice", "wrongpass");
    EXPECT_FALSE(token.has_value());
}

TEST_F(AuthTest, Login_NonExistentUser_ReturnsNullopt)
{
    auto token = auth->login("nobody", "password123");
    EXPECT_FALSE(token.has_value());
}

TEST_F(AuthTest, Login_EmptyUsername_ReturnsNullopt)
{
    auto token = auth->login("", "password123");
    EXPECT_FALSE(token.has_value());
}

TEST_F(AuthTest, Login_EmptyPassword_ReturnsNullopt)
{
    auto token = auth->login("alice", "");
    EXPECT_FALSE(token.has_value());
}

TEST_F(AuthTest, Login_GeneratesUniqueTokens)
{
    auth->signup("alice", "password123", "alice@mail.com");
    auto t1 = auth->login("alice", "password123");
    auto t2 = auth->login("alice", "password123");
    ASSERT_TRUE(t1.has_value());
    ASSERT_TRUE(t2.has_value());
    EXPECT_NE(t1.value(), t2.value());
}

TEST_F(AuthTest, ValidateSession_ValidToken_ReturnsUsername)
{
    auth->signup("alice", "password123", "alice@mail.com");
    auto token    = auth->login("alice", "password123");
    auto username = auth->validateSession(token.value());
    EXPECT_TRUE(username.has_value());
    EXPECT_EQ(username.value(), "alice");
}

TEST_F(AuthTest, ValidateSession_InvalidToken_ReturnsNullopt)
{
    auto username = auth->validateSession("badtoken");
    EXPECT_FALSE(username.has_value());
}

TEST_F(AuthTest, ValidateSession_EmptyToken_ReturnsNullopt)
{
    auto username = auth->validateSession("");
    EXPECT_FALSE(username.has_value());
}

TEST_F(AuthTest, Logout_ValidToken_ReturnsTrue)
{
    auth->signup("alice", "password123", "alice@mail.com");
    auto token = auth->login("alice", "password123");
    EXPECT_TRUE(auth->logout(token.value()));
}

TEST_F(AuthTest, Logout_InvalidatesSession)
{
    auth->signup("alice", "password123", "alice@mail.com");
    auto token = auth->login("alice", "password123");
    auth->logout(token.value());
    auto username = auth->validateSession(token.value());
    EXPECT_FALSE(username.has_value());
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
// Entry Point
// ============================================================================

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}