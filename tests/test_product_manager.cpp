#include <gtest/gtest.h>
#include "../src/include/dbb.hpp"
#include "../src/include/product_manager.hpp"

class ProductManagerTest : public ::testing::Test {
protected:
    Database db{"online-store", "postgres", "1234", "localhost", 5432};
    ProductManager* productManager;

    int test_product_id = 101;
    int test_product_id_no_reviews = 102;

    void SetUp() override {
        db.connect();

        pqxx::work txn(*db.getConnection());

        txn.exec("DELETE FROM review_images");
        txn.exec("DELETE FROM reviews");
        txn.exec("DELETE FROM product_variants");
        txn.exec("DELETE FROM product_images");
        txn.exec("DELETE FROM products");
        txn.exec("DELETE FROM brands");
        txn.exec("DELETE FROM categories");
        txn.commit();

        pqxx::work txn2(*db.getConnection());
        txn2.exec("INSERT INTO brands (id, name) VALUES (1, 'Test Brand')");
        txn2.exec("INSERT INTO categories (id, name, parent_id) VALUES (1, 'Category1', NULL)");
        txn2.exec("INSERT INTO categories (id, name, parent_id) VALUES (2, 'ParentCategory', NULL)");

        txn2.exec_params("INSERT INTO products (id, name, price, brand_id, category_id) VALUES ($1, $2, $3, $4, $5)",
                         test_product_id, "Test Product", 123.45, 1, 1);
        txn2.exec_params("INSERT INTO product_images (product_id, image_url, is_main) VALUES ($1, $2, $3)",
                         test_product_id, "/static/img/test_product.png", true);
        txn2.exec_params("INSERT INTO product_variants (product_id, sku, stock_quantity) VALUES ($1, $2, $3)",
                         test_product_id, "SKU12345", 10);

        txn2.exec_params("INSERT INTO products (id, name, price) VALUES ($1, $2, $3)",
                         test_product_id_no_reviews, "Product No Reviews", 50.0);

        txn2.exec_params("INSERT INTO reviews (id, product_id, user_name, review_text, rating, review_date) VALUES "
                         "(1, $1, 'Alice', 'Great product!', 5, NOW()), "
                         "(2, $1, 'Bob', 'Not bad', 4, NOW())",
                         test_product_id);

        txn2.exec_params("INSERT INTO review_images (review_id, image_url) VALUES (1, '/static/img/review1.png')");
        txn2.exec_params("INSERT INTO review_images (review_id, image_url) VALUES (1, '/static/img/review2.png')");
        txn2.commit();

        productManager = new ProductManager(db);
    }

    void TearDown() override {
        delete productManager;
    }
};

TEST_F(ProductManagerTest, GetProducts_ReturnsNonEmptyVector) {
    auto products = productManager->getProducts(10);
    ASSERT_FALSE(products.empty());
    EXPECT_LE(products.size(), 10);

    for (const auto& p : products) {
        EXPECT_GT(p.id, 0);
        EXPECT_FALSE(p.name.empty());
        EXPECT_FALSE(p.brand.empty());
        EXPECT_FALSE(p.image_url.empty());
        EXPECT_GT(p.price, 0);
    }
}

TEST_F(ProductManagerTest, GetProductById_ExistingProduct_ReturnsCorrectProduct) {
    auto product = productManager->getProductById(test_product_id);

    EXPECT_EQ(product.id, test_product_id);
    EXPECT_EQ(product.name, "Test Product");
    EXPECT_EQ(product.brand, "Test Brand");
    EXPECT_EQ(product.image_url, "/static/img/test_product.png");
    EXPECT_EQ(product.sku, "SKU12345");
    EXPECT_EQ(product.stock_quantity, 10);
    EXPECT_EQ(product.category, "Category1");

    EXPECT_FALSE(product.images.empty());
    EXPECT_EQ(product.images[0].url, "/static/img/test_product.png");
    EXPECT_TRUE(product.images[0].is_main);
}

TEST_F(ProductManagerTest, GetProductById_NonExistingProduct_ReturnsEmptyProduct) {
    auto product = productManager->getProductById(-1);
    EXPECT_EQ(product.id, 0);
    EXPECT_TRUE(product.name.empty());
}

TEST_F(ProductManagerTest, GetReviewsByProduct_NoReviews_ReturnsEmptyVector) {
    auto reviews = productManager->getReviewsByProduct(test_product_id_no_reviews);
    EXPECT_TRUE(reviews.empty());
}
