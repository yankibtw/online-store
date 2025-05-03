#ifndef DB_HPP
#define DB_HPP

#include <pqxx/pqxx>
#include <string>
#include <optional>

class Database {
public:
    Database(const std::string& db_name, const std::string& db_user,
             const std::string& db_password, const std::string& db_host, int db_port);
    ~Database();

    bool connect();
    bool registerUser(const std::string& firstName, const std::string& lastName,
                      const std::string& email, const std::string& phone,
                      const std::string& password);
    std::optional<std::string> Database::authenticateUser(const std::string& email, const std::string& password);
    std::string createSession(const std::string& user_id);
    bool checkSession(const std::string& session_id);
    bool deleteSession(const std::string& session_id);

private:
    std::string generateSessionId();
    std::string hashPassword(const std::string& password);

    std::string db_name_, db_user_, db_password_, db_host_;
    int db_port_;
    pqxx::connection* conn_;
};

#endif
