#include "include/product_manager.hpp"
#include "include/dbb.hpp"

ProductManager::ProductManager(Database& db) : db_(db) {}

std::vector<Product> ProductManager::getProducts(int limit) {
    std::vector<Product> products;
    try {
        pqxx::work W(*db_.getConnection());
        pqxx::result r = W.exec_params(
            "SELECT p.id, p.name, "
            "COALESCE(b.name, 'Без бренда') AS brand, "
            "COALESCE(pi.image_url, '/static/img/default.png') AS image_url, "
            "p.price, "
            "COALESCE(c.name, 'Без категории') AS category, "
            "COALESCE(pc.name, 'Без категории') AS parent_category, "
            "COALESCE(pv.sku, '') AS sku, "
            "COALESCE(pv.stock_quantity, 0) AS stock_quantity "
            "FROM products p "
            "LEFT JOIN brands b ON p.brand_id = b.id "
            "LEFT JOIN product_images pi ON p.id = pi.product_id AND pi.is_main = true "
            "LEFT JOIN categories c ON p.category_id = c.id "
            "LEFT JOIN categories pc ON c.parent_id = pc.id "
            "LEFT JOIN LATERAL ("
            "   SELECT sku, stock_quantity FROM product_variants "
            "   WHERE product_id = p.id LIMIT 1"
            ") pv ON true "
            "LIMIT $1", limit);

        for (auto row : r) {
            Product p;
            p.id = row["id"].as<int>();
            p.name = row["name"].as<std::string>();
            p.brand = row["brand"].as<std::string>();
            p.image_url = row["image_url"].as<std::string>();
            p.price = row["price"].as<double>();
            p.category = row["category"].as<std::string>();
            p.parent_category = row["parent_category"].as<std::string>();
            p.sku = row["sku"].as<std::string>();
            p.stock_quantity = row["stock_quantity"].as<int>();  
            products.push_back(p);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error fetching products: " << e.what() << std::endl;
    }
    return products;
}

Product ProductManager::getProductById(int id) {
    Product p;
    try {
        pqxx::work W(*db_.getConnection());

        pqxx::result r = W.exec_params(
            "SELECT p.id, p.name, "
            "COALESCE(b.name, 'Без бренда') AS brand, "
            "COALESCE(pi.image_url, '/static/img/default.png') AS image_url, "
            "p.price, "
            "COALESCE(p.discount_price, 0.0) AS discount_price, "
            "COALESCE(p.description, '') AS description, "
            "COALESCE(pv.sku, '') AS sku, "
            "COALESCE(pv.stock_quantity, 0) AS stock_quantity, "
            "COALESCE(c.name, 'Без категории') AS category, "
            "COALESCE(pc.name, 'Без категории') AS parent_category "
            "FROM products p "
            "LEFT JOIN brands b ON p.brand_id = b.id "
            "LEFT JOIN product_images pi ON p.id = pi.product_id AND pi.is_main = true "
            "LEFT JOIN categories c ON p.category_id = c.id "
            "LEFT JOIN categories pc ON c.parent_id = pc.id "
            "LEFT JOIN LATERAL ("
            "   SELECT sku, stock_quantity FROM product_variants "
            "   WHERE product_id = p.id LIMIT 1"
            ") pv ON true "
            "WHERE p.id = $1 "
            "LIMIT 1", id
        );

        if (!r.empty()) {
            const auto& row = r[0];
            p.id = row["id"].as<int>();
            p.name = row["name"].as<std::string>();
            p.brand = row["brand"].as<std::string>();
            p.image_url = row["image_url"].as<std::string>();
            p.price = row["price"].as<double>();
            p.discount_price = row["discount_price"].as<double>();
            p.description = row["description"].as<std::string>();
            p.sku = row["sku"].as<std::string>();
            p.stock_quantity = row["stock_quantity"].as<int>();
            p.category = row["category"].as<std::string>();
            p.parent_category = row["parent_category"].as<std::string>();
        } else {
            return p;
        }

        pqxx::result images_res = W.exec_params(
            "SELECT image_url, is_main FROM product_images WHERE product_id = $1 ORDER BY is_main DESC, id ASC",
            id
        );

        p.images.clear();
        for (const auto& img_row : images_res) {
            Image img;
            img.url = img_row["image_url"].as<std::string>();
            img.is_main = img_row["is_main"].as<bool>();
            p.images.push_back(img);
        }

        if (p.image_url.empty() && !p.images.empty()) {
            p.image_url = p.images[0].url;
        }

        W.commit();
    } catch (const std::exception& e) {
        std::cerr << "Error fetching product by ID: " << e.what() << std::endl;
    }
    return p;
}

std::vector<crow::json::wvalue> ProductManager::getReviewsByProduct(int productId) {
    std::vector<crow::json::wvalue> reviews;
    std::unordered_map<int, size_t> reviewIndexMap;

    try {
        pqxx::work txn(*db_.getConnection());

        pqxx::result res = txn.exec_params(
            R"(
                SELECT r.id, r.user_name, r.review_text, r.selected_size, r.rating, r.review_date,
                       ri.image_url
                FROM reviews r
                LEFT JOIN review_images ri ON r.id = ri.review_id
                WHERE r.product_id = $1
                ORDER BY r.review_date DESC, r.id, ri.id
            )",
            productId
        );

        for (const auto& row : res) {
            int reviewId = row["id"].as<int>();

            if (reviewIndexMap.find(reviewId) == reviewIndexMap.end()) {
                crow::json::wvalue review;
                review["userName"] = row["user_name"].c_str();
                review["reviewText"] = row["review_text"].c_str();
                review["selectedSize"] = row["selected_size"].is_null() ? "" : row["selected_size"].c_str();
                review["rating"] = row["rating"].as<int>();
                review["reviewDate"] = row["review_date"].c_str();
                review["images"] = crow::json::wvalue::list();

                reviewIndexMap[reviewId] = reviews.size();
                reviews.push_back(std::move(review));
            }

            if (!row["image_url"].is_null()) {
                size_t index = reviewIndexMap[reviewId];
                auto& images = reviews[index]["images"];

                if (images.size() < 8) {
                    images[images.size()] = row["image_url"].c_str();
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "getReviewsByProduct error: " << e.what() << std::endl;
    }

    return reviews;
}
