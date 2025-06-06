#include "include/dbb.hpp"

Database::Database(const std::string& db_name, const std::string& db_user,
                   const std::string& db_password, const std::string& db_host, int db_port)
    : db_name_(db_name), db_user_(db_user), db_password_(db_password),
      db_host_(db_host), db_port_(db_port), conn_(nullptr) {}

Database::~Database() {
    if (conn_) {
        delete conn_;
    }
}

std::string Database::extractSessionId(const std::string& cookieHeader) {
    std::istringstream ss(cookieHeader);
    std::string token;
    while (std::getline(ss, token, ';')) {
        auto pos = token.find('=');
        if (pos != std::string::npos) {
            std::string key = token.substr(0, pos);
            std::string value = token.substr(pos + 1);
            while (!key.empty() && key.front() == ' ') key.erase(0, 1);
            if (key == "session_id") return value;
        }
    }
    return "";
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

pqxx::connection* Database::getConnection() {
    if (!conn_ || !conn_->is_open()) {
        throw std::runtime_error("Connection is not established or has been closed.");
    }
    return conn_;
}