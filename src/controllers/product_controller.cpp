#include "../include/controllers/product_controller.hpp"

void ProductController::setup(crow::SimpleApp& app, ProductManager& db) {
    CROW_ROUTE(app, "/api/products").methods("GET"_method)([&db]() {
        auto products = db.getProducts(100);
        crow::json::wvalue result = crow::json::wvalue::list(products.size());
        for (size_t i = 0; i < products.size(); ++i)
            result[i] = products[i].toJson();
        return crow::response(result);
    });

    CROW_ROUTE(app, "/api/product/<int>").methods("GET"_method)([&db](int id) {
        Product p = db.getProductById(id);
        if (p.id != 0)
            return crow::response(p.toJson());
        else
            return crow::response(404, R"({"error":"Product not found"})");
    });

    CROW_ROUTE(app, "/api/reviews/<int>").methods("GET"_method)
    ([&db](int productId) {
        std::vector<crow::json::wvalue> reviews = db.getReviewsByProduct(productId);

        if (reviews.empty()) {
            return crow::response(404, "Reviews not found");
        }

        crow::json::wvalue result;
        result["reviews"] = crow::json::wvalue::list(reviews.size());
        for (size_t i = 0; i < reviews.size(); ++i) {
            result["reviews"][i] = std::move(reviews[i]);
        }

        return crow::response(result);
    });
}