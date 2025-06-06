#include <gtest/gtest.h>
#include "../src/include/dbb.hpp"
#include "../src/include/cart_manager.hpp"

class CartManagerTest : public ::testing::Test {
protected:
    Database db{"online-store", "postgres", "1234", "localhost", 5432};
    CartManager* cartManager;

    std::string test_session_id = "test-session";
    int test_user_id = 1;
    int test_product_id = 101;
    std::string test_size = "M";

    void SetUp() override {
        db.connect();

        pqxx::work txn(*db.getConnection());
        txn.exec("DELETE FROM cart_items");
        txn.exec("DELETE FROM sessions");
        txn.exec("DELETE FROM users");
        txn.exec("DELETE FROM products");
        txn.exec("DELETE FROM product_images");
        txn.exec("DELETE FROM product_variants");
        txn.commit();

        pqxx::work txn2(*db.getConnection());
        txn2.exec_params("INSERT INTO users (id, first_name, last_name, email, phone, password) VALUES ($1, $2, $3, $4, $5, $6)",
                         test_user_id, "Test", "User", "test@example.com", "+1234567890", "password");
        txn2.exec_params("INSERT INTO sessions (session_id, user_id) VALUES ($1, $2)", test_session_id, test_user_id);

        txn2.exec_params("INSERT INTO products (id, name, price, discount_price) VALUES ($1, $2, $3, $4)", 
                         test_product_id, "Test Product", 100.0, nullptr);
        txn2.exec_params("INSERT INTO product_images (product_id, image_url, is_main) VALUES ($1, $2, $3)",
                         test_product_id, "/static/img/test.png", true);
        txn2.exec_params("INSERT INTO product_variants (product_id, sku) VALUES ($1, $2)", test_product_id, "SKU12345");
        txn2.commit();

        cartManager = new CartManager(db);
    }

    void TearDown() override {
        delete cartManager;
    }
};

TEST_F(CartManagerTest, AddProductToCart_NewItem) {
    cartManager->addProductToCart(test_session_id, test_product_id, test_size);

    pqxx::work txn(*db.getConnection());
    auto res = txn.exec_params("SELECT quantity FROM cart_items WHERE user_id=$1 AND product_id=$2 AND size=$3",
                               test_user_id, test_product_id, test_size);

    ASSERT_EQ(res.size(), 1);
    EXPECT_EQ(res[0]["quantity"].as<int>(), 1);
}

TEST_F(CartManagerTest, AddProductToCart_ExistingItem_IncrementsQuantity) {
    pqxx::work txn(*db.getConnection());
    txn.exec_params("INSERT INTO cart_items (user_id, product_id, size, quantity, created_at, updated_at) VALUES ($1, $2, $3, 2, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)",
                    test_user_id, test_product_id, test_size);
    txn.commit();

    cartManager->addProductToCart(test_session_id, test_product_id, test_size);

    pqxx::work txn2(*db.getConnection());
    auto res = txn2.exec_params("SELECT quantity FROM cart_items WHERE user_id=$1 AND product_id=$2 AND size=$3",
                                test_user_id, test_product_id, test_size);

    ASSERT_EQ(res.size(), 1);
    EXPECT_EQ(res[0]["quantity"].as<int>(), 3);
}

TEST_F(CartManagerTest, GetCartBySessionId_ReturnsCartItems) {
    pqxx::work txn(*db.getConnection());
    txn.exec_params("INSERT INTO cart_items (user_id, product_id, size, quantity, created_at, updated_at) VALUES ($1, $2, $3, 2, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)",
                    test_user_id, test_product_id, test_size);
    txn.commit();

    auto cartItems = cartManager->getCartBySessionId(test_session_id);

    ASSERT_FALSE(cartItems.empty());
    EXPECT_EQ(cartItems[0].product_id, test_product_id);
    EXPECT_EQ(cartItems[0].quantity, 2);
    EXPECT_EQ(cartItems[0].size, test_size);
    EXPECT_EQ(cartItems[0].name, "Test Product");
    EXPECT_EQ(cartItems[0].image_url, "/static/img/test.png");
    EXPECT_EQ(cartItems[0].sku, "SKU12345");
}

TEST_F(CartManagerTest, RemoveFromCart_RemovesItemSuccessfully) {
    pqxx::work txn(*db.getConnection());
    txn.exec_params("INSERT INTO cart_items (id, user_id, product_id, size, quantity, created_at, updated_at) VALUES (999, $1, $2, $3, 1, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)",
                    test_user_id, test_product_id, test_size);
    txn.commit();

    bool removed = cartManager->removeFromCart(test_session_id, 999);
    EXPECT_TRUE(removed);

    pqxx::work txn2(*db.getConnection());
    auto res = txn2.exec_params("SELECT * FROM cart_items WHERE id=999");
    EXPECT_EQ(res.size(), 0);
}

TEST_F(CartManagerTest, RemoveFromCart_InvalidSession_ReturnsFalse) {
    bool removed = cartManager->removeFromCart("invalid-session", 999);
    EXPECT_FALSE(removed);
}

TEST_F(CartManagerTest, UpdateCartItemQuantity_ChangesQuantity) {
    pqxx::work txn(*db.getConnection());
    txn.exec_params("INSERT INTO cart_items (id, user_id, product_id, size, quantity, created_at, updated_at) VALUES (555, $1, $2, $3, 1, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)",
                    test_user_id, test_product_id, test_size);
    txn.commit();

    cartManager->updateCartItemQuantity(555, 10);

    pqxx::work txn2(*db.getConnection());
    auto res = txn2.exec_params("SELECT quantity FROM cart_items WHERE id=555");
    ASSERT_EQ(res.size(), 1);
    EXPECT_EQ(res[0]["quantity"].as<int>(), 10);
}
