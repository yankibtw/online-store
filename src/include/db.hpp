#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <pqxx/pqxx>
#include <string>

class Database {
public:
    Database(const std::string& db_name, const std::string& db_user,
             const std::string& db_password, const std::string& db_host = "localhost", 
             int db_port = 5432);
    ~Database();

    bool connect();

    pqxx::result executeQuery(const std::string& query);

private:
    std::string db_name_;
    std::string db_user_;
    std::string db_password_;
    std::string db_host_;
    int db_port_;

    pqxx::connection* conn_;
};

#endif
