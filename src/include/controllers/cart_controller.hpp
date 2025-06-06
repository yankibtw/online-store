#pragma once
#ifndef CART_CONTROLLER_HPP
#define CART_CONTROLLER_HPP

#include "..\cart_manager.hpp"
#include "..\user_manager.hpp"

class CartController {
public:
    static void setup(crow::SimpleApp& app, CartManager& db, UserManager& userManager);
};

#endif