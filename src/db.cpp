#include "include/db.hpp"
#include <sodium.h>
#include <iostream>

Database::Database(const std::string& db_name, const std::string& db_user,
                   const std::string& db_password, const std::string& db_host, int db_port)
    : db_name_(db_name), db_user_(db_user), db_password_(db_password), 
      db_host_(db_host), db_port_(db_port), conn_(nullptr) {}

Database::~Database() {
    if (conn_) {
        delete conn_;
    }
}

bool Database::connect() {
    try {
        std::string connection_string = "dbname=" + db_name_ +
                                        " user=" + db_user_ + 
                                        " password=" + db_password_ + 
                                        " host=" + db_host_ + 
                                        " port=" + std::to_string(db_port_);
        
        conn_ = new pqxx::connection(connection_string);
        
        if (conn_->is_open()) {
            std::cout << "Connected to database: " << db_name_ << std::endl;
            return true;
        } else {
            std::cerr << "Failed to connect to database!" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error connecting to database: " << e.what() << std::endl;
    }
    return false;
}

pqxx::result Database::executeQuery(const std::string& query) {
    pqxx::work txn(*conn_);
    pqxx::result result = txn.exec(query);
    txn.commit();
    return result;
}

bool Database::registerUser(const std::string& firstName, const std::string& lastName,
    const std::string& email, const std::string& phone,
    const std::string& password) {

    try {
        pqxx::work txn(*conn_);

        pqxx::result check = txn.exec_params(
            "SELECT id FROM users WHERE email = $1 OR phone = $2",
            email.empty() ? nullptr : email,
            phone.empty() ? nullptr : phone
        );

        if (!check.empty()) {
            std::cerr << "User with this email or phone already exists." << std::endl;
            return false;
        }

        std::string query = "INSERT INTO users (first_name, last_name, email, phone, password) "
            "VALUES ($1, $2, $3, $4, $5)";

        std::string hashedPassword = hashPassword(password);
        
        txn.exec_params(query, 
            firstName, 
            lastName,
            email.empty() ? nullptr : email,
            phone.empty() ? nullptr : phone,
            hashedPassword
        );
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error registering user: " << e.what() << std::endl;
        return false;
    }
}

bool Database::authenticateUser(const std::string& email, const std::string& password) {
    try {
        pqxx::work txn(*conn_);

        pqxx::result result = txn.exec_params(
            "SELECT id, password FROM users WHERE email = $1", email
        );

        if (result.empty()) {
            std::cerr << "User not found" << std::endl;
            return false;
        }

        std::string hashedPassword = result[0]["password"].as<std::string>();

        if (crypto_pwhash_str_verify(hashedPassword.c_str(), password.c_str(), password.length()) != 0) {
            std::cerr << "Invalid password" << std::endl;
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error authenticating user: " << e.what() << std::endl;
        return false;
    }
}

std::string Database::hashPassword(const std::string& password) {
    unsigned char hash[crypto_pwhash_STRBYTES];
    
    if (crypto_pwhash_str(
        reinterpret_cast<char*>(hash), password.c_str(), password.length(),
        crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        throw std::runtime_error("Password hashing failed");
    }
    
    return std::string(reinterpret_cast<char*>(hash));
}

