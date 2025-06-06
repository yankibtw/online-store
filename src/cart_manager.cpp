#include "include/cart_manager.hpp"
#include "include/dbb.hpp"

CartManager::CartManager(Database& db) : db_(db) {}

void CartManager::addProductToCart(const std::string& session_id, int product_id, const std::string& size) {
    try {
        pqxx::work txn(*db_.getConnection());

        pqxx::result user_res = txn.exec_params(
            "SELECT user_id FROM sessions WHERE session_id = $1", session_id
        );

        if (user_res.empty()) {
            throw std::runtime_error("Сессия не найдена");
        }

        std::string user_id = user_res[0]["user_id"].as<std::string>();

        pqxx::result exists_res = txn.exec_params(
            "SELECT id FROM cart_items WHERE user_id = $1 AND product_id = $2 AND size = $3",
            user_id, product_id, size
        );

        if (!exists_res.empty()) {
            txn.exec_params(
                "UPDATE cart_items SET quantity = quantity + 1, updated_at = CURRENT_TIMESTAMP "
                "WHERE user_id = $1 AND product_id = $2 AND size = $3",
                user_id, product_id, size
            );
        } else {
            txn.exec_params(
                "INSERT INTO cart_items (user_id, product_id, size, quantity, created_at, updated_at) "
                "VALUES ($1, $2, $3, 1, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)",
                user_id, product_id, size
            );
        }

        txn.commit();
        std::cout << "Товар успешно добавлен в корзину." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Ошибка при добавлении товара в корзину: " << e.what() << std::endl;
    }
}

std::vector<CartProduct> CartManager::getCartBySessionId(const std::string& session_id) {
    pqxx::work W(*db_.getConnection());

    pqxx::result user_result = W.exec_params(
        "SELECT user_id FROM sessions WHERE session_id = $1",
        session_id
    );

    if (user_result.empty()) {
        throw std::runtime_error("Сессия не найдена");
    }

    int user_id = user_result[0]["user_id"].as<int>();

    pqxx::result r = W.exec_params(R"(
        SELECT
            ci.id AS cart_item_id,
            p.id AS product_id,
            p.name,
            p.price,
            p.discount_price,
            COALESCE(pi.image_url, '/static/img/default.png') AS image_url,
            ci.quantity,
            ci.size,
            pv.sku
        FROM cart_items ci
        JOIN products p ON ci.product_id = p.id
        LEFT JOIN product_images pi ON p.id = pi.product_id AND pi.is_main = true
        LEFT JOIN product_variants pv ON p.id = pv.product_id
        WHERE ci.user_id = $1
    )", user_id);    

    std::vector<CartProduct> items;

    for (const auto& row : r) {
        CartProduct item;
        item.cart_item_id = row["cart_item_id"].as<int>();
        item.product_id = row["product_id"].as<int>();
        item.name = row["name"].as<std::string>();
        item.price = row["price"].as<double>();

        if (row["discount_price"].is_null()) {
            item.discount_price = std::nullopt;
        } else {
            item.discount_price = row["discount_price"].as<double>();
        }

        item.size = row["size"].as<std::string>();
        item.image_url = row["image_url"].as<std::string>();
        item.quantity = row["quantity"].as<int>(); 
        item.sku = row["sku"].as<std::string>(); 

        items.push_back(item);
    }

    return items;
}

bool CartManager::removeFromCart(const std::string& session_id, int cart_item_id) {
    try {
        pqxx::work W(*db_.getConnection());
        
        pqxx::result r = W.exec_params(
            "SELECT user_id FROM sessions WHERE session_id = $1", session_id
        );

        if (r.empty()) {
            return false;
        }

        int user_id = r[0]["user_id"].as<int>();
        W.exec_params(
            "DELETE FROM cart_items WHERE user_id = $1 AND id = $2",
            user_id, cart_item_id
        );

        W.commit();
        
        return true;  
    } catch (const std::exception& e) {
        std::cerr << "Error removing from cart: " << e.what() << std::endl;
        return false; 
    }
}

void CartManager::updateCartItemQuantity(int cart_item_id, int quantity) {
    pqxx::work W(*db_.getConnection());
    W.exec_params("UPDATE cart_items SET quantity = $1 WHERE id = $2", quantity, cart_item_id);
    W.commit();
}
