#ifndef DB_HPP
#define DB_HPP

#include <pqxx/pqxx>
#include <string>

class Database {
public:
    Database(const std::string& db_name, const std::string& db_user,
             const std::string& db_password, const std::string& db_host, int db_port);
    ~Database();
    
    bool connect();
    pqxx::result executeQuery(const std::string& query);
    bool registerUser(const std::string& firstName, const std::string& lastName,
                     const std::string& email, const std::string& phone,
                     const std::string& password);

    bool authenticateUser(const std::string& email, const std::string& password); // Новый метод для аутентификации
    
private:
    std::string db_name_;
    std::string db_user_;
    std::string db_password_;
    std::string db_host_;
    int db_port_;

    pqxx::connection* conn_;
    std::string hashPassword(const std::string& password);
};

#endif
