#pragma once
#ifndef WISHLIST_MANAGER_HPP
#define WISHLIST_MANAGER_HPP

#include "dbb.hpp"
#include <string>
#include <vector>

class WishlistManager {
public:
    explicit WishlistManager(Database& db);

    bool addToFavorites(const std::string& session_id, int product_id);
    std::vector<Product> getFavoritesBySessionId(const std::string& session_id);
    bool removeFromFavorites(const std::string& session_id, int product_id);

private:
    Database& db_;
};

#endif