#pragma once
#ifndef AUTH_CONTROLLER_HPP
#define AUTH_CONTROLLER_HPP

#include "..\user_manager.hpp"

class AuthController {
public:
    static void setup(crow::SimpleApp& app, UserManager& db);
};

#endif