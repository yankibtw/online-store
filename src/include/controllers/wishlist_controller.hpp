#pragma once
#ifndef WISHLIST_CONTROLLER_HPP
#define WISHLIST_CONTROLLER_HPP

#include "..\wishlist_manager.hpp"
#include "..\user_manager.hpp"

class WishlistController {
public:
    static void setup(crow::SimpleApp& app, WishlistManager& db, UserManager& userManager);
};

#endif
