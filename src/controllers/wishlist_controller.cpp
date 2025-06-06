#include "../include/controllers/wishlist_controller.hpp"
#include "../include/controllers/cookie_utils.hpp"
#include <sstream>

void WishlistController::setup(crow::SimpleApp& app, WishlistManager& db, UserManager& userManager) {
    CROW_ROUTE(app, "/api/favorites/add/<int>").methods("POST"_method)([&db, &userManager](const crow::request& req, int productId) {
        std::string session_id = extractSessionId(req.get_header_value("Cookie"));
        if (session_id.empty() || !userManager.checkSession(session_id)) {
            return crow::response(401, R"({"error": "Unauthorized"})");
        }

        bool success = db.addToFavorites(session_id, productId);
        return success ? crow::response(200, R"({"message": "Added to favorites"})")
                       : crow::response(500, R"({"error": "Failed to add to favorites"})");
    });

    CROW_ROUTE(app, "/api/favorites/remove/<int>").methods("POST"_method)([&db, &userManager](const crow::request& req, int productId) {
        std::string session_id = extractSessionId(req.get_header_value("Cookie"));
        if (session_id.empty() || !userManager.checkSession(session_id)) {
            return crow::response(401, R"({"error": "Unauthorized"})");
        }

        bool success = db.removeFromFavorites(session_id, productId);
        return success ? crow::response(200, R"({"message": "Removed from favorites"})")
                       : crow::response(500, R"({"error": "Failed to remove from favorites"})");
    });

    CROW_ROUTE(app, "/api/favorites").methods("GET"_method)([&db, &userManager](const crow::request& req) {
        std::string session_id = extractSessionId(req.get_header_value("Cookie"));
        if (session_id.empty() || !userManager.checkSession(session_id)) {
            return crow::response(401, R"({"error": "Unauthorized"})");
        }

        try {
            auto products = db.getFavoritesBySessionId(session_id);
            crow::json::wvalue result = crow::json::wvalue::list(products.size());
            for (size_t i = 0; i < products.size(); ++i) {
                result[i]["id"] = products[i].id;
                result[i]["name"] = products[i].name;
                result[i]["brand"] = products[i].brand;
                result[i]["price"] = products[i].price;
                result[i]["discount_price"] = products[i].discount_price ? products[i].discount_price : crow::json::wvalue(nullptr);
                result[i]["description"] = products[i].description;
                result[i]["category"] = products[i].category;
                result[i]["parent_category"] = products[i].parent_category;
            }
            return crow::response(result.dump());
        } catch (const std::exception& e) {
            return crow::response(500, R"({"error": ")" + std::string(e.what()) + "\"}");
        }
    });
}