#include <gtest/gtest.h>
#include <sodium.h>
#include "../src/include/dbb.hpp"
#include "../src/include/user_manager.hpp"

class UserManagerTest : public ::testing::Test {
protected:
    Database db{"online-store", "postgres", "1234", "localhost", 5432};
    UserManager userManager{db};

    std::string test_email = "test@example.com";
    std::string test_password = "password123";
    std::string test_first_name = "John";
    std::string test_last_name = "Doe";
    std::string test_phone = "+123456789";

    void SetUp() override {
        db.connect();
        pqxx::work txn(*db.getConnection());
        txn.exec("DELETE FROM sessions");
        txn.exec("DELETE FROM users");
        txn.commit();
    }
};

class SodiumInitEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        if (sodium_init() < 0) {
            throw std::runtime_error("libsodium initialization failed");
        }
    }
};


TEST_F(UserManagerTest, RegisterUser_Success) {
    bool result = userManager.registerUser(test_first_name, test_last_name, test_email, test_phone, test_password);
    EXPECT_TRUE(result);

    pqxx::work W(*db.getConnection());
    pqxx::result r = W.exec_params("SELECT * FROM users WHERE email = $1", test_email);
    EXPECT_EQ(r.size(), 1);
    EXPECT_EQ(r[0]["first_name"].as<std::string>(), test_first_name);
}

TEST_F(UserManagerTest, RegisterUser_EmailAlreadyExists_Failure) {
    ASSERT_TRUE(userManager.registerUser(test_first_name, test_last_name, test_email, test_phone, test_password));
    bool result = userManager.registerUser("Jane", "Smith", test_email, "+987654321", "newpass");
    EXPECT_FALSE(result);

    pqxx::work txn(*db.getConnection());
    pqxx::result r = txn.exec_params("SELECT COUNT(*) FROM users WHERE email = $1", test_email);
    int count = r[0][0].as<int>();
    EXPECT_EQ(count, 1);
}


TEST_F(UserManagerTest, IsEmailAlreadyRegistered) {
    EXPECT_FALSE(userManager.isEmailAlreadyRegistered(test_email));
    userManager.registerUser(test_first_name, test_last_name, test_email, test_phone, test_password);
    EXPECT_TRUE(userManager.isEmailAlreadyRegistered(test_email));
}

TEST_F(UserManagerTest, AuthenticateUser_Success) {
    userManager.registerUser(test_first_name, test_last_name, test_email, test_phone, test_password);

    bool userNotFound = false;
    auto userIdOpt = userManager.authenticateUser(test_email, test_password, userNotFound);
    EXPECT_FALSE(userNotFound);
    EXPECT_TRUE(userIdOpt.has_value());
}

TEST_F(UserManagerTest, AuthenticateUser_WrongPassword) {
    userManager.registerUser(test_first_name, test_last_name, test_email, test_phone, test_password);

    bool userNotFound = false;
    auto userIdOpt = userManager.authenticateUser(test_email, "wrongpassword", userNotFound);
    EXPECT_FALSE(userNotFound);
    EXPECT_FALSE(userIdOpt.has_value());
}

TEST_F(UserManagerTest, AuthenticateUser_UserNotFound) {
    bool userNotFound = false;
    auto userIdOpt = userManager.authenticateUser("notexist@example.com", "any", userNotFound);
    EXPECT_TRUE(userNotFound);
    EXPECT_FALSE(userIdOpt.has_value());
}

TEST_F(UserManagerTest, CreateSession_Success) {
    userManager.registerUser(test_first_name, test_last_name, test_email, test_phone, test_password);

    bool userNotFound = false;
    auto userIdOpt = userManager.authenticateUser(test_email, test_password, userNotFound);
    ASSERT_TRUE(userIdOpt.has_value());

    std::string session_id = userManager.createSession(userIdOpt.value());
    EXPECT_FALSE(session_id.empty());

    pqxx::work W(*db.getConnection());
    pqxx::result r = W.exec_params("SELECT * FROM sessions WHERE session_id = $1", session_id);
    EXPECT_EQ(r.size(), 1);
}

TEST_F(UserManagerTest, CheckSession) {
    userManager.registerUser(test_first_name, test_last_name, test_email, test_phone, test_password);
    bool userNotFound = false;
    auto userIdOpt = userManager.authenticateUser(test_email, test_password, userNotFound);
    ASSERT_TRUE(userIdOpt.has_value());

    std::string session_id = userManager.createSession(userIdOpt.value());
    EXPECT_TRUE(userManager.checkSession(session_id));
    EXPECT_FALSE(userManager.checkSession("invalid-session-id"));
}

TEST_F(UserManagerTest, DeleteSession) {
    userManager.registerUser(test_first_name, test_last_name, test_email, test_phone, test_password);
    bool userNotFound = false;
    auto userIdOpt = userManager.authenticateUser(test_email, test_password, userNotFound);
    ASSERT_TRUE(userIdOpt.has_value());

    std::string session_id = userManager.createSession(userIdOpt.value());
    EXPECT_TRUE(userManager.deleteSession(session_id));
    EXPECT_FALSE(userManager.checkSession(session_id));
}

TEST_F(UserManagerTest, GetUsernameBySession_Success) {
    userManager.registerUser(test_first_name, test_last_name, test_email, test_phone, test_password);
    bool userNotFound = false;
    auto userIdOpt = userManager.authenticateUser(test_email, test_password, userNotFound);
    ASSERT_TRUE(userIdOpt.has_value());

    std::string session_id = userManager.createSession(userIdOpt.value());

    auto usernameOpt = userManager.getUsernameBySession(session_id);
    ASSERT_TRUE(usernameOpt.has_value());
    EXPECT_EQ(usernameOpt.value(), test_first_name + " " + test_last_name);
}

TEST_F(UserManagerTest, GetUsernameBySession_NotFound) {
    auto usernameOpt = userManager.getUsernameBySession("non-existent-session");
    EXPECT_FALSE(usernameOpt.has_value());
}
