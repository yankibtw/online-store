#include "include/order_manager.hpp"

OrderManager::OrderManager(Database& db) : db_(db) {}

bool OrderManager::createOrder(const std::string& session_id, const std::string& payment_method, const std::string& address) {
    try {
        pqxx::work txn(*db_.getConnection());

        pqxx::result user_res = txn.exec_params("SELECT user_id FROM sessions WHERE session_id = $1", session_id);
        if (user_res.empty()) {
            std::cerr << "Invalid session id: " << session_id << std::endl;
            throw std::runtime_error("Invalid session");
        }
        int user_id = user_res[0]["user_id"].as<int>();

        pqxx::result cart_res = txn.exec_params(R"(
            SELECT ci.product_id, ci.size, ci.quantity, COALESCE(p.discount_price, p.price) AS price
            FROM cart_items ci
            JOIN products p ON ci.product_id = p.id
            WHERE ci.user_id = $1
        )", user_id);

        if (cart_res.empty()) {
            std::cerr << "Cart is empty for user_id: " << user_id << std::endl;
            throw std::runtime_error("Cart is empty");
        }

        pqxx::result order_res = txn.exec_params(
            "INSERT INTO orders (user_id, payment_method, status, address) VALUES ($1, $2, 'В обработке', $3) RETURNING id",
            user_id, payment_method, address
        );
        int order_id = order_res[0]["id"].as<int>();

        for (const auto& row : cart_res) {
            int product_id = row["product_id"].as<int>();
            std::string size = row["size"].is_null() ? "" : row["size"].as<std::string>();
            int quantity = row["quantity"].as<int>();
            double price = row["price"].as<double>();

            txn.exec_params(
                "INSERT INTO order_items (order_id, product_id, size, quantity, price) VALUES ($1, $2, $3, $4, $5)",
                order_id, product_id, size, quantity, price
            );
        }

        txn.exec_params("DELETE FROM cart_items WHERE user_id = $1", user_id);
        txn.commit();

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error creating order: " << e.what() << std::endl;
        return false;
    }
}

bool OrderManager::addReview(int product_id,
                         const std::string& user_name,
                         const std::string& review_date,
                         const std::string& review_text,
                         const std::string& selected_size,
                         int rating,
                         const std::vector<std::string>& image_urls) {
    try {
        pqxx::work txn(*db_.getConnection());

        pqxx::result r = txn.exec_params(
            "INSERT INTO reviews (product_id, user_name, review_date, review_text, selected_size, rating) "
            "VALUES ($1, $2, $3, $4, $5, $6) RETURNING id",
            product_id, user_name, review_date, review_text, selected_size, rating
        );

        int review_id = r[0][0].as<int>();

        for (const auto& url : image_urls) {
            txn.exec_params(
                "INSERT INTO review_images (review_id, image_url) VALUES ($1, $2)",
                review_id, url
            );
        }

        txn.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error adding review: " << e.what() << std::endl;
        return false;
    }
}

std::vector<Order> OrderManager::getOrderHistory(const std::string& session_id) {
    pqxx::work txn(*db_.getConnection());

    pqxx::result user_result = txn.exec_params(
        "SELECT user_id FROM sessions WHERE session_id = $1", session_id
    );

    if (user_result.empty()) {
        throw std::runtime_error("Сессия не найдена");
    }

    int user_id = user_result[0]["user_id"].as<int>();

    pqxx::result order_res = txn.exec_params(R"(
        SELECT id, payment_method, status, address, created_at
        FROM orders
        WHERE user_id = $1
        ORDER BY created_at DESC
    )", user_id);

    std::vector<Order> orders;

    for (const auto& row : order_res) {
        Order order;
        order.order_id = row["id"].as<int>();
        order.payment_method = row["payment_method"].as<std::string>();
        order.status = row["status"].as<std::string>();
        order.address = row["address"].as<std::string>();
        order.created_at = row["created_at"].as<std::string>();

        pqxx::result items_res = txn.exec_params(R"(
            SELECT 
                oi.product_id,
                p.name,
                oi.size,
                oi.quantity,
                oi.price,
                COALESCE(p.discount_price, NULL) AS discount_price,
                COALESCE(pi.image_url, '/static/img/default.png') AS image_url,
                pv.sku
            FROM order_items oi
            JOIN products p ON oi.product_id = p.id
            LEFT JOIN product_images pi ON p.id = pi.product_id AND pi.is_main = true
            LEFT JOIN product_variants pv ON p.id = pv.product_id
            WHERE oi.order_id = $1
        )", order.order_id);

        for (const auto& item_row : items_res) {
            OrderItem item;
            item.product_id = item_row["product_id"].as<int>();
            item.name = item_row["name"].as<std::string>();
            item.size = item_row["size"].as<std::string>();
            item.quantity = item_row["quantity"].as<int>();
            item.price = item_row["price"].as<double>();

            if (item_row["discount_price"].is_null()) {
                item.discount_price = std::nullopt;
            } else {
                item.discount_price = item_row["discount_price"].as<double>();
            }

            item.image_url = item_row["image_url"].as<std::string>();
            item.sku = item_row["sku"].as<std::string>();

            order.items.push_back(item);
        }

        orders.push_back(order);
    }

    return orders;
}
