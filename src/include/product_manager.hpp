#pragma once
#ifndef PRODUCT_MANAGER_HPP
#define PRODUCT_MANAGER_HPP

#include "dbb.hpp"
#include <vector>

class ProductManager {
public:
    explicit ProductManager(Database& db);

    std::vector<Product> getProducts(int limit);
    Product getProductById(int id);
    std::vector<crow::json::wvalue> getReviewsByProduct(int productId);

private:
    Database& db_;
};

#endif