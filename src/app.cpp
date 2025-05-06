#include "include\app.hpp"
#include "..\include\crow\include\crow_all.h"
#include <sstream>
#include <iostream>

std::string extractSessionId(const std::string& cookieHeader) {
    std::istringstream ss(cookieHeader);
    std::string token;
    while (std::getline(ss, token, ';')) {
        auto pos = token.find('=');
        if (pos != std::string::npos) {
            std::string key = token.substr(0, pos);
            std::string value = token.substr(pos + 1);
            while (!key.empty() && key.front() == ' ') key.erase(0, 1);
            if (key == "session_id") {
                return value;
            }
        }
    }
    return "";
}

void setupRoutes(crow::SimpleApp& app, Database& db) {
    CROW_ROUTE(app, "/")([]() {
        return crow::mustache::load("main.html").render();
    });

    CROW_ROUTE(app, "/catalog")([]() {
        return crow::mustache::load("catalog.html").render();
    });

    CROW_ROUTE(app, "/card")([]() {
        return crow::mustache::load("card.html").render();
    });

    CROW_ROUTE(app, "/basket")([]() {
        return crow::mustache::load("basket.html").render();
    });

    CROW_ROUTE(app, "/favourite")([]() {
        return crow::mustache::load("favourite.html").render();
    });

    CROW_ROUTE(app, "/register").methods("POST"_method)
    ([&db](const crow::request& req) {
        auto x = crow::json::load(req.body);
        if (!x) {
            return crow::response(400, crow::json::wvalue{{"error", "Invalid JSON"}}.dump());
        }

        std::string firstName = x["firstName"].s();
        std::string lastName = x["lastName"].s();
        std::string email = x["email"].s();
        std::string phone = x["phone"].s();
        std::string password = x["password"].s();

        if (firstName.empty() || lastName.empty() || password.empty()) {
            return crow::response(400, crow::json::wvalue{{"error", "First name, last name, and password are required"}}.dump());
        }

        if (db.isEmailAlreadyRegistered(email)) {
            return crow::response(400, crow::json::wvalue{{"error", "Email is already registered"}}.dump());
        }

        if (db.registerUser(firstName, lastName, email, phone, password)) {
            bool userNotFound = false;
            auto userIdOpt = db.authenticateUser(email, password, userNotFound);

            if (userIdOpt) {
                std::string sessionId = db.createSession(*userIdOpt);
                if (!sessionId.empty()) {
                    crow::json::wvalue response;
                    response["message"] = "Registration and login successful";
                    response["sessionId"] = sessionId;

                    crow::response res(200);
                    res.set_header("Set-Cookie", "session_id=" + sessionId + "; Path=/; HttpOnly; SameSite=Lax");
                    res.write(response.dump());
                    res.set_header("Content-Type", "application/json");

                    return res;
                } else {
                    return crow::response(500, crow::json::wvalue{{"error", "Failed to create session"}}.dump());
                }
            } else {
                return crow::response(500, crow::json::wvalue{{"error", "Authentication failed"}}.dump());
            }
        } else {
            return crow::response(500, crow::json::wvalue{{"error", "Registration failed"}}.dump());
        }
    });

    CROW_ROUTE(app, "/login").methods("POST"_method)
    ([&db](const crow::request& req) {
        auto x = crow::json::load(req.body);
        if (!x) {
            return crow::response(400, crow::json::wvalue{{"error", "Invalid JSON"}});
        }

        auto email = x["email"].s();
        auto password = x["password"].s();

        bool userNotFound = false;
        auto userIdOpt = db.authenticateUser(email, password, userNotFound);

        if (userIdOpt.has_value()) {
            std::string sessionId = db.createSession(*userIdOpt);
            if (!sessionId.empty()) {
                crow::response res(200);
                res.set_header("Set-Cookie", "session_id=" + sessionId + "; Path=/; HttpOnly; SameSite=Lax");
                res.write(crow::json::wvalue{{"message", "Login successful"}, {"sessionId", sessionId}}.dump());
                return res;
            }
        }

        if (userNotFound) {
            return crow::response(404, crow::json::wvalue{{"error", "User not found"}});
        }

        return crow::response(401, crow::json::wvalue{{"error", "Invalid password"}});
    });

    CROW_ROUTE(app, "/logout").methods("POST"_method)
    ([&db](const crow::request& req) {
        std::string raw_cookie = req.get_header_value("Cookie");
        std::string session_id = extractSessionId(raw_cookie);
        if (!session_id.empty()) {
            db.deleteSession(session_id);
            return crow::response(200, crow::json::wvalue{{"message", "Logged out"}});
        }
        return crow::response(400, crow::json::wvalue{{"error", "No session"}});
    });

    CROW_ROUTE(app, "/api/products").methods("GET"_method)
    ([&db]() {
        auto products = db.getProducts();
        crow::json::wvalue result;

        result = crow::json::wvalue::list(products.size());

        for (size_t i = 0; i < products.size(); ++i) {
            result[i] = products[i].toJson();
        }

        return crow::response(result);
    });

    CROW_ROUTE(app, "/api/product/<int>")
    .methods("GET"_method)
    ([&db](int productId) {
        Product product = db.getProductById(productId);
        if (product.id != 0) {
            return crow::response(product.toJson());
        } else {
            return crow::response(404, "Product not found");
        }
    });    

    CROW_ROUTE(app, "/api/reviews/<int>").methods("GET"_method)
    ([&db](int productId) {
        std::vector<crow::json::wvalue> reviews = db.getReviewsByProduct(productId);

        if (reviews.empty()) {
            return crow::response(404, "Отзывы не найдены");
        }

        crow::json::wvalue result;
        result["reviews"] = crow::json::wvalue::list(reviews.size());
        for (size_t i = 0; i < reviews.size(); ++i) {
            result["reviews"][i] = std::move(reviews[i]);
        }

        return crow::response(result);
    });

    CROW_ROUTE(app, "/api/favorites/add/<int>").methods("POST"_method)
    ([&db](const crow::request& req, int productId) {
        std::string session_id = extractSessionId(req.get_header_value("Cookie"));
        
        if (session_id.empty() || !db.checkSession(session_id)) {
            return crow::response(401, crow::json::wvalue{{"error", "Unauthorized"}});
        }

        if (db.addToFavorites(session_id, productId)) {
            return crow::response(200, crow::json::wvalue{{"message", "Added to favorites"}});
        } else {
            return crow::response(500, crow::json::wvalue{{"error", "Failed to add to favorites"}});
        }
    });

    CROW_ROUTE(app, "/api/favorites").methods("GET"_method)
    ([&db](const crow::request& req) {
        std::string session_id = extractSessionId(req.get_header_value("Cookie"));
    
        if (session_id.empty()) {
            return crow::response(401, "Unauthorized");
        }
    
        std::vector<Product> favoriteProducts = db.getFavoritesBySessionId(session_id);
    
        if (favoriteProducts.empty()) {
            return crow::response(200, "[]");
        }
    
        crow::json::wvalue response = crow::json::wvalue::list(favoriteProducts.size()); 
        for (size_t i = 0; i < favoriteProducts.size(); ++i) {
            const auto& product = favoriteProducts[i];
            
            response[i] = crow::json::wvalue{
                {"id", product.id},
                {"name", product.name},
                {"brand", product.brand},
                {"image_url", product.image_url},
                {"price", product.price}
            };
        }
    
        return crow::response{response};
    });
 
    CROW_ROUTE(app, "/api/favorites/remove/<int>").methods("POST"_method)
    ([&db](const crow::request& req, int productId) {
        std::string session_id = extractSessionId(req.get_header_value("Cookie"));
        
        if (session_id.empty() || !db.checkSession(session_id)) {
            return crow::response(401, crow::json::wvalue{{"error", "Unauthorized"}}); 
        }

        if (db.removeFromFavorites(session_id, productId)) {
            return crow::response(200, crow::json::wvalue{{"message", "Product removed from favorites"}}); 
        } else {
            return crow::response(500, crow::json::wvalue{{"error", "Failed to remove product from favorites"}});
        }
    });

    CROW_ROUTE(app, "/api/cart/add/<int>").methods("POST"_method)
    ([&db](const crow::request& req, int productId) {
        std::string session_id = extractSessionId(req.get_header_value("Cookie"));
        
        if (session_id.empty() || !db.checkSession(session_id)) {
            return crow::response(401, crow::json::wvalue{{"error", "Unauthorized"}});
        }
    
        try {
            db.addProductToCart(session_id, productId);
            return crow::response(200, crow::json::wvalue{{"message", "Added to cart"}});
        } catch (const std::exception& e) {
            std::string error_msg = e.what();
            if (error_msg.find("в корзине") != std::string::npos) {
                return crow::response(409, crow::json::wvalue{{"error", "Already in cart"}});
            }
            return crow::response(500, crow::json::wvalue{{"error", error_msg}});
        }
    });
    
    CROW_ROUTE(app, "/api/cart").methods("GET"_method)
    ([&db](const crow::request& req) {
        std::string session_id = extractSessionId(req.get_header_value("Cookie"));
    
        if (session_id.empty()) {
            return crow::response(401, "Unauthorized");
        }
    
        std::vector<CartProduct> cartProducts;
        try {
            cartProducts = db.getCartBySessionId(session_id);
        } catch (const std::exception& e) {
            return crow::response(500, std::string("Ошибка получения корзины: ") + e.what());
        }
    
        if (cartProducts.empty()) {
            crow::json::wvalue emptyResponse;
            emptyResponse["status"] = "empty";
            return crow::response{emptyResponse};
        }
    
        crow::json::wvalue response = crow::json::wvalue::list(cartProducts.size());
        for (size_t i = 0; i < cartProducts.size(); ++i) {
            const auto& product = cartProducts[i];
    
            response[i] = crow::json::wvalue{
                {"cart_item_id", product.cart_item_id},
                {"product_id", product.product_id},
                {"name", product.name},
                {"price", product.price},
                {"discount_price", product.discount_price.has_value() ? crow::json::wvalue(*product.discount_price) : crow::json::wvalue(nullptr)},
                {"image_url", product.image_url},
                {"quantity", product.quantity} 
            };
        }
    
        return crow::response{response};
    });
    
    CROW_ROUTE(app, "/api/cart/remove/<int>").methods("POST"_method)
    ([&db](const crow::request& req, int item_id) {
        std::string session_id = extractSessionId(req.get_header_value("Cookie"));
        
        if (session_id.empty()) {
            return crow::response(401, "Unauthorized");
        }

        if (db.removeFromCart(session_id, item_id)) {
            return crow::response(200);  
        } else {
            return crow::response(500, "Failed to remove from cart"); 
        }
    });
    
    CROW_ROUTE(app, "/api/cart/update_quantity/<int>").methods("POST"_method)
    ([&db](const crow::request& req, int cart_item_id) {
        crow::json::rvalue body;
        try {
            body = crow::json::load(req.body);
            if (!body || !body.has("quantity")) {
                return crow::response(400, "Missing quantity field");
            }
            int new_quantity = body["quantity"].i();
    
            if (new_quantity <= 0) {
                return crow::response(400, "Quantity must be positive");
            }
    
            db.updateCartItemQuantity(cart_item_id, new_quantity);
            return crow::response(200);
        } catch (const std::exception& e) {
            return crow::response(500, std::string("Ошибка при обновлении количества: ") + e.what());
        }
    });
}