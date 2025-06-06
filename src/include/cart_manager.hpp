#pragma once
#ifndef CART_MANAGER_HPP
#define CART_MANAGER_HPP

#include "dbb.hpp"
#include <string>
#include <vector>

class CartManager {
public:
    explicit CartManager(Database& db);

    void addProductToCart(const std::string& session_id, int product_id, const std::string& size);
    std::vector<CartProduct> getCartBySessionId(const std::string& session_id);
    bool removeFromCart(const std::string& session_id, int product_id);
    void updateCartItemQuantity(int cart_item_id, int quantity);

private:
    Database& db_;
};

#endif 