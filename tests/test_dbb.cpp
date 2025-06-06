#include <gtest/gtest.h>
#include "../src/include/dbb.hpp"

TEST(DatabaseTest, ExtractSessionId_Valid) {
    std::string cookieHeader = "user=admin; session_id=abc123xyz; token=secure";
    EXPECT_EQ(Database::extractSessionId(cookieHeader), "abc123xyz");
}

TEST(DatabaseTest, ExtractSessionId_Whitespace) {
    std::string cookieHeader = "session_id = valueWithSpaces ; token=something";
    EXPECT_EQ(Database::extractSessionId(cookieHeader), "valueWithSpaces"); 
}

TEST(DatabaseTest, ExtractSessionId_Missing) {
    std::string cookieHeader = "user=admin; token=abc";
    EXPECT_EQ(Database::extractSessionId(cookieHeader), "");
}

TEST(DatabaseTest, GenerateSessionId_Length) {
    Database db("", "", "", "", 0);
    std::string sid = db.generateSessionId();
    EXPECT_EQ(sid.length(), 32);
}

TEST(DatabaseTest, GenerateSessionId_Unique) {
    Database db("", "", "", "", 0);
    std::string sid1 = db.generateSessionId();
    std::string sid2 = db.generateSessionId();
    EXPECT_NE(sid1, sid2);
}

TEST(DatabaseTest, ConnectToDatabase) {
    Database db("online-store", "postgres", "1234", "localhost", 5432);
    bool connected = db.connect();
    EXPECT_TRUE(connected);

    if (connected) {
        pqxx::connection* conn = db.getConnection();
        EXPECT_TRUE(conn->is_open());
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
