#include "../include/controllers/cart_controller.hpp"
#include "../include/controllers/cookie_utils.hpp"
#include <sstream>

void CartController::setup(crow::SimpleApp& app, CartManager& db, UserManager& userManager) {
    CROW_ROUTE(app, "/api/cart/add/<int>").methods("POST"_method)([&db, &userManager](const crow::request& req, int productId) {
        std::string session_id = extractSessionId(req.get_header_value("Cookie"));
        if (session_id.empty() || !userManager.checkSession(session_id)) {
            return crow::response(401, R"({"error": "Unauthorized"})");
        }

        auto body = crow::json::load(req.body);
        if (!body || !body.has("size")) {
            return crow::response(400, R"({"error": "Size is required"})");
        }

        try {
            db.addProductToCart(session_id, productId, body["size"].s());
            return crow::response(200, R"({"message": "Added to cart"})");
        } catch (const std::exception& e) {
            return crow::response(500, crow::json::wvalue{{"error", e.what()}}.dump());
        }
    });

    CROW_ROUTE(app, "/api/cart").methods("GET"_method)([&db](const crow::request& req) {
        std::string session_id = extractSessionId(req.get_header_value("Cookie"));
        if (session_id.empty()) {
            return crow::response(401, "Unauthorized");
        }

        try {
            auto cart = db.getCartBySessionId(session_id);
            crow::json::wvalue result = crow::json::wvalue::list(cart.size());
            for (size_t i = 0; i < cart.size(); ++i) {
                const auto& item = cart[i];
                result[i]["cart_item_id"] = item.cart_item_id;
                result[i]["product_id"] = item.product_id;
                result[i]["name"] = item.name;
                result[i]["price"] = item.price;
                result[i]["discount_price"] = item.discount_price ? *item.discount_price : crow::json::wvalue(nullptr);
                result[i]["image_url"] = item.image_url;
                result[i]["quantity"] = item.quantity;
                result[i]["size"] = item.size;
                result[i]["sku"] = item.sku;
            }
            return crow::response(result.dump());
        } catch (const std::exception& e) {
            return crow::response(500, R"({"error": ")" + std::string(e.what()) + "\"}");
        }
    });

    CROW_ROUTE(app, "/api/cart/remove/<int>").methods("POST"_method)([&db, &userManager](const crow::request& req, int cartItemId) {
        std::string session_id = extractSessionId(req.get_header_value("Cookie"));
        if (session_id.empty() || !userManager.checkSession(session_id)) {
            return crow::response(401, "Unauthorized");
        }

        bool success = db.removeFromCart(session_id, cartItemId);
        return success ? crow::response(200, R"({"message": "Removed from cart"})")
                       : crow::response(500, R"({"error": "Failed to remove from cart"})");
    });

    CROW_ROUTE(app, "/api/cart/update_quantity/<int>").methods("POST"_method)([&db](const crow::request& req, int cart_item_id) {
        auto body = crow::json::load(req.body);
        if (!body || !body.has("quantity")) {
            return crow::response(400, R"({"error": "Missing quantity field"})");
        }

        int new_quantity = body["quantity"].i();
        if (new_quantity <= 0) {
            return crow::response(400, R"({"error": "Quantity must be positive"})");
        }

        try {
            db.updateCartItemQuantity(cart_item_id, new_quantity);
            return crow::response(200, R"({"message": "Quantity updated"})");
        } catch (const std::exception& e) {
            return crow::response(500, R"({"error": ")" + std::string(e.what()) + "\"}");
        }
    });

    CROW_ROUTE(app, "/api/checkout/products").methods("POST"_method)
    ([&db](const crow::request& req) {
        std::string session_id = extractSessionId(req.get_header_value("Cookie"));

        if (session_id.empty()) {
            return crow::response(401, "Unauthorized");
        }

        auto body = crow::json::load(req.body);
        if (!body || body["product_ids"].t() != crow::json::type::List) {
            return crow::response(400, "Invalid JSON: product_ids must be a list");
        }

        std::vector<int> selected_ids;
        for (const auto& id : body["product_ids"]) {
            selected_ids.push_back(id.i());
        }

        std::vector<CartProduct> cartProducts;
        try {
            cartProducts = db.getCartBySessionId(session_id);
        } catch (const std::exception& e) {
            return crow::response(500, std::string("Error receiving the shopping cart: ") + e.what());
        }

        std::vector<CartProduct> selectedProducts;
        for (const auto& product : cartProducts) {
            if (std::find(selected_ids.begin(), selected_ids.end(), product.cart_item_id) != selected_ids.end()) {
                selectedProducts.push_back(product);
            }
        }

        crow::json::wvalue response = crow::json::wvalue::list(selectedProducts.size());
        for (size_t i = 0; i < selectedProducts.size(); ++i) {
            const auto& product = selectedProducts[i];
            response[i] = crow::json::wvalue{
                {"cart_item_id", product.cart_item_id},
                {"product_id", product.product_id},
                {"name", product.name},
                {"price", product.price},
                {"discount_price", product.discount_price.has_value() ? crow::json::wvalue(*product.discount_price) : crow::json::wvalue(nullptr)},
                {"image_url", product.image_url},
                {"quantity", product.quantity},
                {"size", product.size},
                {"sku", product.sku}
            };
        }

        return crow::response{response};
    });
}