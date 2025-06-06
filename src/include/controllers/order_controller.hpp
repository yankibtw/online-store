#pragma once
#ifndef ORDER_CONTROLLER_HPP
#define ORDER_CONTROLLER_HPP

#include "..\order_manager.hpp"
#include "..\user_manager.hpp"

class OrderController {
public:
    static void setup(crow::SimpleApp& app, OrderManager& db, UserManager& userManager);
};

#endif 