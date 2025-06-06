#include "../include/controllers/order_controller.hpp"
#include "../include/controllers/cookie_utils.hpp"
#include <sstream>

std::string getCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

void OrderController::setup(crow::SimpleApp& app, OrderManager& db, UserManager& userManager) {
    CROW_ROUTE(app, "/api/order/create").methods("POST"_method)([&db, &userManager](const crow::request& req) {
        std::string session_id = extractSessionId(req.get_header_value("Cookie"));
        if (session_id.empty() || !userManager.checkSession(session_id)) {
            return crow::response(401, R"({"error": "Unauthorized"})");
        }

        auto body = crow::json::load(req.body);
        if (!body || !body.has("payment_method") || !body.has("address")) {
            return crow::response(400, R"({"error": "Missing fields"})");
        }

        bool success = db.createOrder(session_id, body["payment_method"].s(), body["address"].s());
        return success ? crow::response(200, R"({"message": "Order created"})")
                      : crow::response(500, R"({"error": "Failed to create order"})");
    });

    CROW_ROUTE(app, "/api/orders/history")
    .methods("GET"_method)
    ([&db, &userManager](const crow::request& req) {
        std::string session_id = extractSessionId(req.get_header_value("Cookie"));

        if (session_id.empty() || !userManager.checkSession(session_id)) {
            return crow::response(401, crow::json::wvalue{{"error", "Unauthorized"}});
        }

        try {
            std::vector<Order> orders = db.getOrderHistory(session_id);

            crow::json::wvalue result;
            result["orders"] = crow::json::wvalue::list();

            int order_idx = 0;
            for (const auto& order : orders) {
                crow::json::wvalue order_json;
                order_json["order_id"] = order.order_id;
                order_json["payment_method"] = order.payment_method;
                order_json["status"] = order.status;
                order_json["address"] = order.address;
                order_json["created_at"] = order.created_at;

                order_json["items"] = crow::json::wvalue::list();
                int item_idx = 0;
                for (const auto& item : order.items) {
                    crow::json::wvalue item_json;
                    item_json["product_id"] = item.product_id;
                    item_json["name"] = item.name;
                    item_json["size"] = item.size;
                    item_json["quantity"] = item.quantity;
                    item_json["price"] = item.price;

                    if (item.discount_price.has_value()) {
                        item_json["discount_price"] = *item.discount_price;
                    } else {
                        item_json["discount_price"] = nullptr;
                    }

                    item_json["image_url"] = item.image_url;
                    item_json["sku"] = item.sku;

                    order_json["items"][item_idx++] = std::move(item_json);
                }

                result["orders"][order_idx++] = std::move(order_json);
            }

            return crow::response(200, result);

        } catch (const std::exception& e) {
            return crow::response(500, crow::json::wvalue{{"error", std::string("Exception: ") + e.what()}});
        }
    });

        CROW_ROUTE(app, "/api/reviews/add/<int>").methods("POST"_method)
    ([&db, &userManager](const crow::request& req, int productId) {
        std::string session_id = extractSessionId(req.get_header_value("Cookie"));

        if (session_id.empty() || !userManager.checkSession(session_id)) {
            return crow::response(401, crow::json::wvalue{{"error", "Unauthorized"}});
        }

        auto body = crow::json::load(req.body);
        if (!body || !body.has("rating") || !body.has("comment")) {
            return crow::response(400, crow::json::wvalue{{"error", "Missing rating or comment"}});
        }

        int rating = body["rating"].i();
        std::string comment = body["comment"].s();
        
        std::string selected_size = "";
        if (body.has("selected_size")) {
            selected_size = body["selected_size"].s();
        }

        std::vector<std::string> image_urls;
        if (body.has("image_urls") && body["image_urls"].t() == crow::json::type::List) {
            for (auto& img : body["image_urls"]) {
                image_urls.push_back(img.s());
            }
        }

        if (rating < 1 || rating > 5 || comment.empty()) {
            return crow::response(400, crow::json::wvalue{{"error", "Rating must be 1-5 and comment must not be empty"}});
        }

        try {
            std::optional<std::string> usernameOpt = userManager.getUsernameBySession(session_id);

            if (!usernameOpt) {
                return crow::response(401, crow::json::wvalue{{"error", "Invalid session"}});
            }

            std::string username = *usernameOpt;
            std::string review_date = getCurrentTimestamp();

            bool success = db.addReview(
                productId,
                username,
                review_date,
                comment,
                selected_size,
                rating,
                image_urls
            );

            if (success) {
                return crow::response(200, crow::json::wvalue{{"message", "Review added successfully"}});
            } else {
                return crow::response(500, crow::json::wvalue{{"error", "Failed to add review"}});
            }
        } catch (const std::exception& e) {
            return crow::response(500, crow::json::wvalue{{"error", std::string("Exception: ") + e.what()}});
        }
    });
}