#include "include/db.hpp"
#include <iostream>
#include <cstdlib>
#include <random>
#include <sstream>
#include <iomanip>
#include <sodium.h>

Database::Database(const std::string& db_name, const std::string& db_user,
                   const std::string& db_password, const std::string& db_host, int db_port)
    : db_name_(db_name), db_user_(db_user), db_password_(db_password),
      db_host_(db_host), db_port_(db_port), conn_(nullptr) {}

Database::~Database() {
    if (conn_) {
        delete conn_;
    }
}

std::string Database::generateSessionId() {
    std::stringstream ss;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    for (int i = 0; i < 16; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
    }
    return ss.str();
}

bool Database::connect() {
    try {
        std::string connection_string = "dbname=" + db_name_ + " user=" + db_user_ + 
                                        " password=" + db_password_ + " host=" + db_host_ + 
                                        " port=" + std::to_string(db_port_);
        conn_ = new pqxx::connection(connection_string);
        return conn_->is_open();
    } catch (const std::exception& e) {
        std::cerr << "Connection failed: " << e.what() << std::endl;
        return false;
    }
}

bool Database::registerUser(const std::string& firstName, const std::string& lastName,
                            const std::string& email, const std::string& phone,
                            const std::string& password) {
    try {
        pqxx::work W(*conn_);
        W.exec_params(
            "INSERT INTO users (first_name, last_name, email, phone, password) VALUES ($1, $2, $3, $4, $5)",
            firstName, lastName, email, phone, hashPassword(password)
        );
        W.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error registering user: " << e.what() << std::endl;
        return false;
    }
}

std::optional<std::string> Database::authenticateUser(const std::string& email, const std::string& password, bool& userNotFound) {
    try {
        pqxx::work W(*conn_);
        pqxx::result r = W.exec_params("SELECT id, password FROM users WHERE email=$1", email);

        if (r.empty()) {
            userNotFound = true;
            return std::nullopt;
        }

        std::string storedPassword = r[0]["password"].as<std::string>();
        if (crypto_pwhash_str_verify(storedPassword.c_str(), password.c_str(), password.size()) != 0) {
            return std::nullopt; 
        }

        return r[0]["id"].as<std::string>();
    } catch (const std::exception& e) {
        std::cerr << "Error authenticating user: " << e.what() << std::endl;
    }
    return std::nullopt;
}

std::string Database::createSession(const std::string& user_id) {
    std::string session_id = generateSessionId();
    try {
        pqxx::work W(*conn_);
        W.exec_params("INSERT INTO sessions (user_id, session_id) VALUES ($1, $2)", user_id, session_id);
        W.commit();
    } catch (const std::exception& e) {
        std::cerr << "Error creating session: " << e.what() << std::endl;
        session_id = "";
    }
    return session_id;
}

bool Database::checkSession(const std::string& session_id) {
    try {
        pqxx::work W(*conn_);
        std::string query = "SELECT user_id FROM sessions WHERE session_id = '" + session_id + "'";
        pqxx::result r = W.exec(query);
        return !r.empty();
    } catch (const std::exception& e) {
        std::cerr << "Error checking session: " << e.what() << std::endl;
        return false;
    }
}

bool Database::deleteSession(const std::string& session_id) {
    try {
        pqxx::work W(*conn_);
        std::string query = "DELETE FROM sessions WHERE session_id = '" + session_id + "'";
        W.exec(query);
        W.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error deleting session: " << e.what() << std::endl;
        return false;
    }
}

std::string Database::hashPassword(const std::string& password) {
    if (sodium_init() < 0) {
        throw std::runtime_error("Failed to initialize libsodium");
    }

    char hashed[crypto_pwhash_STRBYTES];

    if (crypto_pwhash_str(
            hashed,
            password.c_str(),
            password.size(),
            crypto_pwhash_OPSLIMIT_INTERACTIVE,
            crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        throw std::runtime_error("Password hashing failed");
    }

    return std::string(hashed);
}

bool Database::isEmailAlreadyRegistered(const std::string& email) {
    try {
        pqxx::work W(*conn_);
        pqxx::result r = W.exec_params("SELECT 1 FROM users WHERE email=$1", email);
        return !r.empty();
    } catch (const std::exception& e) {
        std::cerr << "Error checking email registration: " << e.what() << std::endl;
        return false; 
    }
}

std::vector<Product> Database::getProducts(int limit) {
    std::vector<Product> products;
    try {
        pqxx::work W(*conn_);
        pqxx::result r = W.exec_params(
            "SELECT p.id, p.name, b.name AS brand, pi.image_url, p.price "
            "FROM products p "
            "LEFT JOIN brands b ON p.brand_id = b.id "
            "LEFT JOIN product_images pi ON p.id = pi.product_id AND pi.is_main = true "
            "LIMIT $1", limit);

        for (auto row : r) {
            Product p;
            p.id = row["id"].as<int>();
            p.name = row["name"].as<std::string>();
            p.brand = row["brand"].is_null() ? "Без бренда" : row["brand"].as<std::string>();
            p.image_url = row["image_url"].is_null() ? "/static/img/default.png" : row["image_url"].as<std::string>();
            p.price = row["price"].as<double>();
            products.push_back(p);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error fetching products: " << e.what() << std::endl;
    }
    return products;
}

Product Database::getProductById(int id) {

    Product p;
    try {
        pqxx::work W(*conn_);
        pqxx::result r = W.exec_params(
            "SELECT p.id, p.name, COALESCE(b.name, 'Без бренда') AS brand, "
            "COALESCE(pi.image_url, '/static/img/default.png') AS image_url, "
            "p.price, COALESCE(p.description, '') AS description "
            "FROM products p "
            "LEFT JOIN brands b ON p.brand_id = b.id "
            "LEFT JOIN product_images pi ON p.id = pi.product_id AND pi.is_main = true "
            "WHERE p.id = $1", id
        );

        if (!r.empty()) {
            const auto& row = r[0];
            p.id = row["id"].as<int>();
            p.name = row["name"].as<std::string>();
            p.brand = row["brand"].as<std::string>();
            p.image_url = row["image_url"].as<std::string>();
            p.price = row["price"].as<double>();
            p.description = row["description"].as<std::string>();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error fetching product by ID: " << e.what() << std::endl;
    }
    return p;
}

std::vector<crow::json::wvalue> Database::getReviewsByProduct(int productId) {
    std::vector<crow::json::wvalue> reviews;
    std::unordered_map<int, size_t> reviewIndexMap;

    try {
        pqxx::work txn(*conn_);

        pqxx::result res = txn.exec_params(
            R"(
                SELECT r.id, r.user_name, r.review_text, r.selected_size, r.rating, r.review_date,
                       ri.image_url
                FROM reviews r
                LEFT JOIN review_images ri ON r.id = ri.review_id
                WHERE r.product_id = $1
                ORDER BY r.review_date DESC, r.id, ri.id
            )",
            productId
        );

        for (const auto& row : res) {
            int reviewId = row["id"].as<int>();

            if (reviewIndexMap.find(reviewId) == reviewIndexMap.end()) {
                crow::json::wvalue review;
                review["userName"] = row["user_name"].c_str();
                review["reviewText"] = row["review_text"].c_str();
                review["selectedSize"] = row["selected_size"].is_null() ? "" : row["selected_size"].c_str();
                review["rating"] = row["rating"].as<int>();
                review["reviewDate"] = row["review_date"].c_str();
                review["images"] = crow::json::wvalue::list();

                reviewIndexMap[reviewId] = reviews.size();
                reviews.push_back(std::move(review));
            }

            if (!row["image_url"].is_null()) {
                size_t index = reviewIndexMap[reviewId];
                auto& images = reviews[index]["images"];

                if (images.size() < 8) {
                    images[images.size()] = row["image_url"].c_str();
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "getReviewsByProduct error: " << e.what() << std::endl;
    }

    return reviews;
}

bool Database::addToFavorites(const std::string& session_id, int product_id) {
    try {
        pqxx::work W(*conn_);
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

std::vector<Product> Database::getFavoritesBySessionId(const std::string& session_id) {
    std::vector<Product> products;
    try {
        pqxx::work W(*conn_);
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

bool Database::removeFromFavorites(const std::string& session_id, int product_id) {
    try {
        pqxx::work W(*conn_);
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
