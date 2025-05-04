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