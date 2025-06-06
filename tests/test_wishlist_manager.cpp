#include <gtest/gtest.h>
#include "../src/include/dbb.hpp"
#include "../src/include/wishlist_manager.hpp"

class WishlistManagerTest : public ::testing::Test {
protected:
    Database db{"online-store", "postgres", "1234", "localhost", 5432};
    WishlistManager* wishlistManager;

    std::string test_session_id = "test-session";
    int test_user_id = 1;
    int test_product_id = 101;

    void SetUp() override {
        db.connect();

        pqxx::work txn(*db.getConnection());

        txn.exec("DELETE FROM favorites");
        txn.exec("DELETE FROM sessions");
        txn.exec("DELETE FROM users");
        txn.exec("DELETE FROM products");
        txn.exec("DELETE FROM brands");
        txn.exec("DELETE FROM product_images");
        txn.commit();

        pqxx::work txn2(*db.getConnection());
        txn2.exec_params("INSERT INTO users (id, first_name, last_name, email, phone, password) VALUES ($1, $2, $3, $4, $5, $6)",
                         test_user_id, "Test", "User", "test@example.com", "+1234567890", "password");
        txn2.exec_params("INSERT INTO sessions (session_id, user_id) VALUES ($1, $2)", test_session_id, test_user_id);

        txn2.exec_params("INSERT INTO brands (id, name) VALUES (1, 'TestBrand')");
        txn2.exec_params("INSERT INTO products (id, name, price, brand_id) VALUES ($1, $2, $3, 1)", 
                         test_product_id, "Test Product", 100.0);
        txn2.exec_params("INSERT INTO product_images (product_id, image_url, is_main) VALUES ($1, $2, true)", 
                         test_product_id, "/static/img/test.png");
        txn2.commit();

        wishlistManager = new WishlistManager(db);
    }

    void TearDown() override {
        delete wishlistManager;
    }
};

TEST_F(WishlistManagerTest, AddToFavorites_Success) {
    bool added = wishlistManager->addToFavorites(test_session_id, test_product_id);
    EXPECT_TRUE(added);

    pqxx::work txn(*db.getConnection());
    auto res = txn.exec_params("SELECT * FROM favorites WHERE user_id=$1 AND product_id=$2", test_user_id, test_product_id);
    EXPECT_EQ(res.size(), 1);
}

TEST_F(WishlistManagerTest, AddToFavorites_InvalidSession_ReturnsFalse) {
    bool added = wishlistManager->addToFavorites("invalid-session", test_product_id);
    EXPECT_FALSE(added);
}

TEST_F(WishlistManagerTest, GetFavoritesBySessionId_ReturnsProducts) {
    pqxx::work txn(*db.getConnection());
    txn.exec_params("INSERT INTO favorites (user_id, product_id) VALUES ($1, $2)", test_user_id, test_product_id);
    txn.commit();

    auto favorites = wishlistManager->getFavoritesBySessionId(test_session_id);
    ASSERT_FALSE(favorites.empty());
    EXPECT_EQ(favorites[0].id, test_product_id);
    EXPECT_EQ(favorites[0].name, "Test Product");
    EXPECT_EQ(favorites[0].brand, "TestBrand");
    EXPECT_EQ(favorites[0].image_url, "/static/img/test.png");
    EXPECT_DOUBLE_EQ(favorites[0].price, 100.0);
}

TEST_F(WishlistManagerTest, GetFavoritesBySessionId_InvalidSession_Throws) {
    EXPECT_THROW({
        wishlistManager->getFavoritesBySessionId("invalid-session");
    }, std::runtime_error);
}

TEST_F(WishlistManagerTest, RemoveFromFavorites_Success) {
    pqxx::work txn(*db.getConnection());
    txn.exec_params("INSERT INTO favorites (user_id, product_id) VALUES ($1, $2)", test_user_id, test_product_id);
    txn.commit();

    bool removed = wishlistManager->removeFromFavorites(test_session_id, test_product_id);
    EXPECT_TRUE(removed);

    pqxx::work txn2(*db.getConnection());
    auto res = txn2.exec_params("SELECT * FROM favorites WHERE user_id=$1 AND product_id=$2", test_user_id, test_product_id);
    EXPECT_EQ(res.size(), 0);
}

TEST_F(WishlistManagerTest, RemoveFromFavorites_InvalidSession_ReturnsFalse) {
    bool removed = wishlistManager->removeFromFavorites("invalid-session", test_product_id);
    EXPECT_FALSE(removed);
}
