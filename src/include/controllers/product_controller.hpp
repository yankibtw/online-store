#pragma once
#ifndef PRODUCT_CONTROLLER_HPP
#define PRODUCT_CONTROLLER_HPP

#include "..\product_manager.hpp"

class ProductController {
public:
    static void setup(crow::SimpleApp& app, ProductManager& db);
};

#endif