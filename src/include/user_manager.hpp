#pragma once
#ifndef USER_MANAGER_HPP
#define USER_MANAGER_HPP

#include "dbb.hpp"
#include <string>
#include <optional>
#include <sodium.h>
#include <stdexcept>

class UserManager {
public:
    explicit UserManager(Database& db);

    bool registerUser(const std::string& firstName, const std::string& lastName,
                      const std::string& email, const std::string& phone,
                      const std::string& password);

    std::optional<std::string> authenticateUser(const std::string& email,
                                                const std::string& password,
                                                bool& userNotFound);

    std::string hashPassword(const std::string& password);
    std::string createSession(const std::string& userId);
    bool checkSession(const std::string& sessionId);
    bool deleteSession(const std::string& sessionId);
    std::optional<std::string> getUsernameBySession(const std::string& sessionId);
    bool isEmailAlreadyRegistered(const std::string& email);

private:
    Database& db_;
};

#endif 