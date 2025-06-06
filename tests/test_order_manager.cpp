#include <gtest/gtest.h>
#include "../src/include/dbb.hpp"
#include "../src/include/order_manager.hpp"

class OrderManagerTest : public ::testing::Test {
protected:
    Database db{"online-store", "postgres", "1234", "localhost", 5432};
    OrderManager* orderManager;

    std::string test_session_id = "test-session";
    int test_user_id = 1;
    int test_product_id = 101;

    void SetUp() override {
        db.connect();

        pqxx::work txn(*db.getConnection());
        txn.exec("DELETE FROM order_items");
        txn.exec("DELETE FROM orders");
        txn.exec("DELETE FROM review_images");
        txn.exec("DELETE FROM reviews");
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

        orderManager = new OrderManager(db);
    }

    void TearDown() override {
        delete orderManager;
    }
};

TEST_F(OrderManagerTest, CreateOrder_Success) {
    pqxx::work txn(*db.getConnection());
    txn.exec_params("INSERT INTO cart_items (user_id, product_id, size, quantity, created_at, updated_at) VALUES ($1, $2, $3, $4, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)",
                    test_user_id, test_product_id, "M", 2);
    txn.commit();

    bool created = orderManager->createOrder(test_session_id, "Credit Card", "123 Test St");
    EXPECT_TRUE(created);

    pqxx::work txn2(*db.getConnection());
    auto cart_res = txn2.exec_params("SELECT * FROM cart_items WHERE user_id=$1", test_user_id);
    EXPECT_EQ(cart_res.size(), 0);

    auto order_res = txn2.exec_params("SELECT * FROM orders WHERE user_id=$1", test_user_id);
    EXPECT_EQ(order_res.size(), 1);

    auto order_items_res = txn2.exec_params("SELECT * FROM order_items WHERE order_id=$1", order_res[0]["id"].as<int>());
    EXPECT_EQ(order_items_res.size(), 1);
    EXPECT_EQ(order_items_res[0]["product_id"].as<int>(), test_product_id);
    EXPECT_EQ(order_items_res[0]["quantity"].as<int>(), 2);
}

TEST_F(OrderManagerTest, CreateOrder_InvalidSession_ReturnsFalse) {
    bool created = orderManager->createOrder("invalid-session", "Credit Card", "123 Test St");
    EXPECT_FALSE(created);
}

TEST_F(OrderManagerTest, CreateOrder_EmptyCart_ReturnsFalse) {
    bool created = orderManager->createOrder(test_session_id, "Credit Card", "123 Test St");
    EXPECT_FALSE(created);
}

TEST_F(OrderManagerTest, AddReview_Success) {
    std::vector<std::string> images = {"/static/img/review1.png", "/static/img/review2.png"};
    bool added = orderManager->addReview(test_product_id, "Tester", "2025-06-06", "Great product!", "M", 5, images);
    EXPECT_TRUE(added);

    pqxx::work txn(*db.getConnection());
    auto reviews = txn.exec_params("SELECT * FROM reviews WHERE product_id=$1 AND user_name=$2", test_product_id, "Tester");
    EXPECT_EQ(reviews.size(), 1);

    int review_id = reviews[0]["id"].as<int>();
    auto images_res = txn.exec_params("SELECT * FROM review_images WHERE review_id=$1", review_id);
    EXPECT_EQ(images_res.size(), (int)images.size());
}

TEST_F(OrderManagerTest, GetOrderHistory_ReturnsOrders) {
    pqxx::work txn(*db.getConnection());
    txn.exec_params("INSERT INTO orders (id, user_id, payment_method, status, address, created_at) VALUES (1, $1, $2, 'В обработке', $3, CURRENT_TIMESTAMP)", test_user_id, "Credit Card", "123 Test St");
    txn.exec_params("INSERT INTO order_items (order_id, product_id, size, quantity, price) VALUES (1, $1, 'M', 3, 100.0)", test_product_id);
    txn.commit();

    auto orders = orderManager->getOrderHistory(test_session_id);

    ASSERT_FALSE(orders.empty());
    EXPECT_EQ(orders[0].order_id, 1);
    EXPECT_EQ(orders[0].payment_method, "Credit Card");
    EXPECT_EQ(orders[0].status, "В обработке");
    EXPECT_EQ(orders[0].address, "123 Test St");
    ASSERT_FALSE(orders[0].items.empty());
    EXPECT_EQ(orders[0].items[0].product_id, test_product_id);
    EXPECT_EQ(orders[0].items[0].quantity, 3);
    EXPECT_EQ(orders[0].items[0].size, "M");
}

TEST_F(OrderManagerTest, GetOrderHistory_InvalidSession_Throws) {
    EXPECT_THROW({
        orderManager->getOrderHistory("invalid-session");
    }, std::runtime_error);
}
