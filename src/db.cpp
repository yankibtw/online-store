#include "include\db.hpp"
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
