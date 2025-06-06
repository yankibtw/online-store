#include "include/user_manager.hpp"

UserManager::UserManager(Database& db) : db_(db) {}


bool UserManager::registerUser(const std::string& firstName, const std::string& lastName,
                               const std::string& email, const std::string& phone,
                               const std::string& password) {
    try {
        pqxx::work W(*db_.getConnection());
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

std::optional<std::string> UserManager::authenticateUser(const std::string& email,
                                                         const std::string& password,
                                                         bool& userNotFound) {
    try {
        pqxx::work W(*db_.getConnection());
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
        return std::nullopt;
    }
}

std::string UserManager::createSession(const std::string& user_id) {
    std::string session_id = db_.generateSessionId();
    try {
        pqxx::work W(*db_.getConnection());
        W.exec_params("INSERT INTO sessions (user_id, session_id) VALUES ($1, $2)", user_id, session_id);
        W.commit();
    } catch (const std::exception& e) {
        std::cerr << "Error creating session: " << e.what() << std::endl;
        session_id = "";
    }
    return session_id;
}

bool UserManager::checkSession(const std::string& session_id) {
    try {
        pqxx::work W(*db_.getConnection());
        pqxx::result r = W.exec("SELECT user_id FROM sessions WHERE session_id = '" + session_id + "'");
        return !r.empty();
    } catch (const std::exception& e) {
        std::cerr << "Error checking session: " << e.what() << std::endl;
        return false;
    }
}

bool UserManager::deleteSession(const std::string& session_id) {
    try {
        pqxx::work W(*db_.getConnection());
        W.exec("DELETE FROM sessions WHERE session_id = '" + session_id + "'");
        W.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error deleting session: " << e.what() << std::endl;
        return false;
    }
}

std::string UserManager::hashPassword(const std::string& password) {
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

bool UserManager::isEmailAlreadyRegistered(const std::string& email) {
    try {
        pqxx::work W(*db_.getConnection());
        pqxx::result r = W.exec_params("SELECT 1 FROM users WHERE email=$1", email);
        return !r.empty();
    } catch (const std::exception& e) {
        std::cerr << "Error checking email registration: " << e.what() << std::endl;
        return false;
    }
}

std::optional<std::string> UserManager::getUsernameBySession(const std::string& session_id) {
    try {
        pqxx::work W(*db_.getConnection());
        pqxx::result session_result = W.exec_params(
            "SELECT user_id FROM sessions WHERE session_id = $1", session_id
        );
        if (session_result.empty()) {
            return std::nullopt;
        }
        std::string user_id = session_result[0]["user_id"].as<std::string>();
        pqxx::result user_result = W.exec_params(
            "SELECT first_name, last_name FROM users WHERE id = $1", user_id
        );
        if (user_result.empty()) {
            return std::nullopt;
        }
        std::string first_name = user_result[0]["first_name"].as<std::string>();
        std::string last_name = user_result[0]["last_name"].as<std::string>();
        return first_name + " " + last_name;
    } catch (const std::exception& e) {
        std::cerr << "Error in getUsernameBySession: " << e.what() << std::endl;
        return std::nullopt;
    }
}