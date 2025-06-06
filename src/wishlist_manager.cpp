#include "include/wishlist_manager.hpp"
#include "include/dbb.hpp"

WishlistManager::WishlistManager(Database& db) : db_(db) {}

bool WishlistManager::addToFavorites(const std::string& session_id, int product_id) {
    try {
        pqxx::work W(*db_.getConnection());
        pqxx::result r = W.exec_params(
            "SELECT user_id FROM sessions WHERE session_id = $1",
            session_id
        );

        if (r.empty()) {
            return false;
        }

        std::string user_id = r[0]["user_id"].as<std::string>();

        W.exec_params(
            "INSERT INTO favorites (user_id, product_id) VALUES ($1, $2) ON CONFLICT DO NOTHING",
            user_id, product_id
        );

        W.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error adding to favorites: " << e.what() << std::endl;
        return false;
    }
}

std::vector<Product> WishlistManager::getFavoritesBySessionId(const std::string& session_id) {
    std::vector<Product> products;
    try {
        pqxx::work W(*db_.getConnection());
        pqxx::result r = W.exec_params(
            "SELECT user_id FROM sessions WHERE session_id = $1",
            session_id
        );

        if (r.empty()) {
            throw std::runtime_error("Сессия не найдена");
        }

        std::string user_id = r[0]["user_id"].as<std::string>();

        r = W.exec_params(
            "SELECT p.id, p.name, COALESCE(b.name, 'Без бренда') AS brand, "
            "COALESCE(pi.image_url, '/static/img/default.png') AS image_url, "
            "p.price "
            "FROM favorites f "
            "JOIN products p ON f.product_id = p.id "
            "LEFT JOIN brands b ON p.brand_id = b.id "
            "LEFT JOIN product_images pi ON p.id = pi.product_id AND pi.is_main = true "
            "WHERE f.user_id = $1",
            user_id
        );

        for (auto row : r) {
            Product p;
            p.id = row["id"].as<int>();
            p.name = row["name"].as<std::string>();
            p.brand = row["brand"].as<std::string>();
            p.image_url = row["image_url"].as<std::string>();
            p.price = row["price"].as<double>();
            products.push_back(p);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error fetching favorite products: " << e.what() << std::endl;
    }
    return products;
}

bool WishlistManager::removeFromFavorites(const std::string& session_id, int product_id) {
    try {
        pqxx::work W(*db_.getConnection());
        pqxx::result r = W.exec_params(
            "SELECT user_id FROM sessions WHERE session_id = $1", session_id
        );

        if (r.empty()) {
            return false; 
        }

        std::string user_id = r[0]["user_id"].as<std::string>();

        W.exec_params(
            "DELETE FROM favorites WHERE user_id = $1 AND product_id = $2",
            user_id, product_id
        );

        W.commit();
        return true; 
    } catch (const std::exception& e) {
        std::cerr << "Error removing from favorites: " << e.what() << std::endl;
        return false; 
    }
}
