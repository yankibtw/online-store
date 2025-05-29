#include "include/db.hpp"
#include <iostream>
#include <cstdlib>
#include <random>
#include <sstream>
#include <iomanip>
#include <sodium.h>

Database::Database(const std::string& db_name, const std::string& db_user,
                   const std::string& db_password, const std::string& db_host, int db_port)
    : db_name_(db_name), db_user_(db_user), db_password_(db_password),
      db_host_(db_host), db_port_(db_port), conn_(nullptr) {}

Database::~Database() {
    if (conn_) {
        delete conn_;
    }
}

std::string Database::generateSessionId() {
    std::stringstream ss;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    for (int i = 0; i < 16; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
    }
    return ss.str();
}

bool Database::connect() {
    try {
        std::string connection_string = "dbname=" + db_name_ + " user=" + db_user_ + 
                                        " password=" + db_password_ + " host=" + db_host_ + 
                                        " port=" + std::to_string(db_port_);
        conn_ = new pqxx::connection(connection_string);
        return conn_->is_open();
    } catch (const std::exception& e) {
        std::cerr << "Connection failed: " << e.what() << std::endl;
        return false;
    }
}

bool Database::registerUser(const std::string& firstName, const std::string& lastName,
                            const std::string& email, const std::string& phone,
                            const std::string& password) {
    try {
        pqxx::work W(*conn_);
        W.exec_params(
            "INSERT INTO users (first_name, last_name, email, phone, password) VALUES ($1, $2, $3, $4, $5)",
            firstName, lastName, email, phone, hashPassword(password)
        );
        W.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error registering user: " << e.what() << std::endl;
        return false;
    }
}

std::optional<std::string> Database::getUsernameBySession(const std::string& session_id) {
    try {
        pqxx::work W(*conn_);

        pqxx::result session_result = W.exec_params(
            "SELECT user_id FROM sessions WHERE session_id = $1", session_id
        );

        if (session_result.empty()) {
            return std::nullopt;
        }

        std::string user_id = session_result[0]["user_id"].as<std::string>();

        pqxx::result user_result = W.exec_params(
            "SELECT first_name, last_name FROM users WHERE id = $1", user_id
        );

        if (user_result.empty()) {
            return std::nullopt;
        }

        std::string first_name = user_result[0]["first_name"].as<std::string>();
        std::string last_name = user_result[0]["last_name"].as<std::string>();

        return first_name + " " + last_name;

    } catch (const std::exception& e) {
        std::cerr << "Error in getUsernameBySession: " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<std::string> Database::authenticateUser(const std::string& email, const std::string& password, bool& userNotFound) {
    try {
        pqxx::work W(*conn_);
        pqxx::result r = W.exec_params("SELECT id, password FROM users WHERE email=$1", email);

        if (r.empty()) {
            userNotFound = true;
            return std::nullopt;
        }

        std::string storedPassword = r[0]["password"].as<std::string>();
        if (crypto_pwhash_str_verify(storedPassword.c_str(), password.c_str(), password.size()) != 0) {
            return std::nullopt; 
        }

        return r[0]["id"].as<std::string>();
    } catch (const std::exception& e) {
        std::cerr << "Error authenticating user: " << e.what() << std::endl;
    }
    return std::nullopt;
}

std::string Database::createSession(const std::string& user_id) {
    std::string session_id = generateSessionId();
    try {
        pqxx::work W(*conn_);
        W.exec_params("INSERT INTO sessions (user_id, session_id) VALUES ($1, $2)", user_id, session_id);
        W.commit();
    } catch (const std::exception& e) {
        std::cerr << "Error creating session: " << e.what() << std::endl;
        session_id = "";
    }
    return session_id;
}

bool Database::checkSession(const std::string& session_id) {
    try {
        pqxx::work W(*conn_);
        std::string query = "SELECT user_id FROM sessions WHERE session_id = '" + session_id + "'";
        pqxx::result r = W.exec(query);
        return !r.empty();
    } catch (const std::exception& e) {
        std::cerr << "Error checking session: " << e.what() << std::endl;
        return false;
    }
}

bool Database::deleteSession(const std::string& session_id) {
    try {
        pqxx::work W(*conn_);
        std::string query = "DELETE FROM sessions WHERE session_id = '" + session_id + "'";
        W.exec(query);
        W.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error deleting session: " << e.what() << std::endl;
        return false;
    }
}

std::string Database::hashPassword(const std::string& password) {
    if (sodium_init() < 0) {
        throw std::runtime_error("Failed to initialize libsodium");
    }

    char hashed[crypto_pwhash_STRBYTES];

    if (crypto_pwhash_str(
            hashed,
            password.c_str(),
            password.size(),
            crypto_pwhash_OPSLIMIT_INTERACTIVE,
            crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        throw std::runtime_error("Password hashing failed");
    }

    return std::string(hashed);
}

bool Database::isEmailAlreadyRegistered(const std::string& email) {
    try {
        pqxx::work W(*conn_);
        pqxx::result r = W.exec_params("SELECT 1 FROM users WHERE email=$1", email);
        return !r.empty();
    } catch (const std::exception& e) {
        std::cerr << "Error checking email registration: " << e.what() << std::endl;
        return false; 
    }
}

std::vector<Product> Database::getProducts(int limit) {
    std::vector<Product> products;
    try {
        pqxx::work W(*conn_);
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

Product Database::getProductById(int id) {
    Product p;
    try {
        pqxx::work W(*conn_);
        
        pqxx::result r = W.exec_params(
            "SELECT p.id, p.name, "
            "COALESCE(b.name, 'Без бренда') AS brand, "
            "COALESCE(pi.image_url, '/static/img/default.png') AS image_url, "
            "p.price, p.discount_price, "
            "COALESCE(p.description, '') AS description, "
            "pv.sku, "
            "COALESCE(c.name, 'Без категории') AS category, "
            "COALESCE(pc.name, 'Без категории') AS parent_category "
            "FROM products p "
            "LEFT JOIN brands b ON p.brand_id = b.id "
            "LEFT JOIN product_images pi ON p.id = pi.product_id AND pi.is_main = true "
            "LEFT JOIN categories c ON p.category_id = c.id "
            "LEFT JOIN categories pc ON c.parent_id = pc.id "
            "LEFT JOIN product_variants pv ON p.id = pv.product_id "
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
            p.sku = row["sku"].is_null() ? "" : row["sku"].as<std::string>();
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


std::vector<crow::json::wvalue> Database::getReviewsByProduct(int productId) {
    std::vector<crow::json::wvalue> reviews;
    std::unordered_map<int, size_t> reviewIndexMap;

    try {
        pqxx::work txn(*conn_);

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

bool Database::addToFavorites(const std::string& session_id, int product_id) {
    try {
        pqxx::work W(*conn_);
        pqxx::result r = W.exec_params(
            "SELECT user_id FROM sessions WHERE session_id = $1",
            session_id
        );

        if (r.empty()) {
            return false;
        }

        std::string user_id = r[0]["user_id"].as<std::string>();

        W.exec_params(
            "INSERT INTO favorites (user_id, product_id) VALUES ($1, $2) ON CONFLICT DO NOTHING",
            user_id, product_id
        );

        W.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error adding to favorites: " << e.what() << std::endl;
        return false;
    }
}

std::vector<Product> Database::getFavoritesBySessionId(const std::string& session_id) {
    std::vector<Product> products;
    try {
        pqxx::work W(*conn_);
        pqxx::result r = W.exec_params(
            "SELECT user_id FROM sessions WHERE session_id = $1",
            session_id
        );

        if (r.empty()) {
            throw std::runtime_error("Сессия не найдена");
        }

        std::string user_id = r[0]["user_id"].as<std::string>();

        r = W.exec_params(
            "SELECT p.id, p.name, COALESCE(b.name, 'Без бренда') AS brand, "
            "COALESCE(pi.image_url, '/static/img/default.png') AS image_url, "
            "p.price "
            "FROM favorites f "
            "JOIN products p ON f.product_id = p.id "
            "LEFT JOIN brands b ON p.brand_id = b.id "
            "LEFT JOIN product_images pi ON p.id = pi.product_id AND pi.is_main = true "
            "WHERE f.user_id = $1",
            user_id
        );

        for (auto row : r) {
            Product p;
            p.id = row["id"].as<int>();
            p.name = row["name"].as<std::string>();
            p.brand = row["brand"].as<std::string>();
            p.image_url = row["image_url"].as<std::string>();
            p.price = row["price"].as<double>();
            products.push_back(p);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error fetching favorite products: " << e.what() << std::endl;
    }
    return products;
}

bool Database::removeFromFavorites(const std::string& session_id, int product_id) {
    try {
        pqxx::work W(*conn_);
        pqxx::result r = W.exec_params(
            "SELECT user_id FROM sessions WHERE session_id = $1", session_id
        );

        if (r.empty()) {
            return false; 
        }

        std::string user_id = r[0]["user_id"].as<std::string>();

        W.exec_params(
            "DELETE FROM favorites WHERE user_id = $1 AND product_id = $2",
            user_id, product_id
        );

        W.commit();
        return true; 
    } catch (const std::exception& e) {
        std::cerr << "Error removing from favorites: " << e.what() << std::endl;
        return false; 
    }
}

void Database::addProductToCart(const std::string& session_id, int product_id, const std::string& size) {
    try {
        pqxx::work txn(*conn_);

        pqxx::result user_res = txn.exec_params(
            "SELECT user_id FROM sessions WHERE session_id = $1", session_id
        );

        if (user_res.empty()) {
            throw std::runtime_error("Сессия не найдена");
        }

        std::string user_id = user_res[0]["user_id"].as<std::string>();

        pqxx::result exists_res = txn.exec_params(
            "SELECT id FROM cart_items WHERE user_id = $1 AND product_id = $2 AND size = $3",
            user_id, product_id, size
        );

        if (!exists_res.empty()) {
            txn.exec_params(
                "UPDATE cart_items SET quantity = quantity + 1, updated_at = CURRENT_TIMESTAMP "
                "WHERE user_id = $1 AND product_id = $2 AND size = $3",
                user_id, product_id, size
            );
        } else {
            txn.exec_params(
                "INSERT INTO cart_items (user_id, product_id, size, quantity, created_at, updated_at) "
                "VALUES ($1, $2, $3, 1, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)",
                user_id, product_id, size
            );
        }

        txn.commit();
        std::cout << "Товар успешно добавлен в корзину." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Ошибка при добавлении товара в корзину: " << e.what() << std::endl;
    }
}

std::vector<CartProduct> Database::getCartBySessionId(const std::string& session_id) {
    pqxx::work W(*conn_);

    pqxx::result user_result = W.exec_params(
        "SELECT user_id FROM sessions WHERE session_id = $1",
        session_id
    );

    if (user_result.empty()) {
        throw std::runtime_error("Сессия не найдена");
    }

    int user_id = user_result[0]["user_id"].as<int>();

    pqxx::result r = W.exec_params(R"(
        SELECT
            ci.id AS cart_item_id,
            p.id AS product_id,
            p.name,
            p.price,
            p.discount_price,
            COALESCE(pi.image_url, '/static/img/default.png') AS image_url,
            ci.quantity,
            ci.size,
            pv.sku
        FROM cart_items ci
        JOIN products p ON ci.product_id = p.id
        LEFT JOIN product_images pi ON p.id = pi.product_id AND pi.is_main = true
        LEFT JOIN product_variants pv ON p.id = pv.product_id
        WHERE ci.user_id = $1
    )", user_id);    

    std::vector<CartProduct> items;

    for (const auto& row : r) {
        CartProduct item;
        item.cart_item_id = row["cart_item_id"].as<int>();
        item.product_id = row["product_id"].as<int>();
        item.name = row["name"].as<std::string>();
        item.price = row["price"].as<double>();

        if (row["discount_price"].is_null()) {
            item.discount_price = std::nullopt;
        } else {
            item.discount_price = row["discount_price"].as<double>();
        }

        item.size = row["size"].as<std::string>();
        item.image_url = row["image_url"].as<std::string>();
        item.quantity = row["quantity"].as<int>(); 
        item.sku = row["sku"].as<std::string>(); 

        items.push_back(item);
    }

    return items;
}

bool Database::removeFromCart(const std::string& session_id, int product_id) {
    try {
        pqxx::work W(*conn_);
        
        pqxx::result r = W.exec_params(
            "SELECT user_id FROM sessions WHERE session_id = $1", session_id
        );

        if (r.empty()) {
            return false;
        }

        std::string user_id = r[0]["user_id"].as<std::string>();
        W.exec_params(
            "DELETE FROM cart_items WHERE user_id = $1 AND id = $2",
            user_id, product_id
        );

        W.commit();
        
        return true;  
    } catch (const std::exception& e) {
        std::cerr << "Error removing from cart: " << e.what() << std::endl;
        return false; 
    }
}

void Database::updateCartItemQuantity(int cart_item_id, int quantity) {
    pqxx::work W(*conn_);
    W.exec_params("UPDATE cart_items SET quantity = $1 WHERE id = $2", quantity, cart_item_id);
    W.commit();
}

bool Database::createOrder(const std::string& session_id, const std::string& payment_method) {
    try {
        pqxx::work txn(*conn_);
        
        pqxx::result user_res = txn.exec_params("SELECT user_id FROM sessions WHERE session_id = $1", session_id);
        if (user_res.empty()) {
            std::cerr << "Invalid session id: " << session_id << std::endl;
            throw std::runtime_error("Invalid session");
        }
        int user_id = user_res[0]["user_id"].as<int>();

        pqxx::result cart_res = txn.exec_params(
            R"(
                SELECT ci.product_id, ci.size, ci.quantity, COALESCE(p.discount_price, p.price) AS price
                FROM cart_items ci
                JOIN products p ON ci.product_id = p.id
                WHERE ci.user_id = $1
            )", user_id
        );

        if (cart_res.empty()) {
            std::cerr << "Cart is empty for user_id: " << user_id << std::endl;
            throw std::runtime_error("Cart is empty");
        }

        pqxx::result order_res = txn.exec_params(
            "INSERT INTO orders (user_id, payment_method, status) VALUES ($1, $2, 'completed') RETURNING id",
            user_id, payment_method
        );

        int order_id = order_res[0]["id"].as<int>();

        for (auto row : cart_res) {
            int product_id = row["product_id"].as<int>();
            std::string size = row["size"].is_null() ? "" : row["size"].as<std::string>();
            int quantity = row["quantity"].as<int>();
            double price = row["price"].as<double>();

            txn.exec_params(
                "INSERT INTO order_items (order_id, product_id, size, quantity, price) VALUES ($1, $2, $3, $4, $5)",
                order_id, product_id, size, quantity, price
            );
        }

        txn.exec_params("DELETE FROM cart_items WHERE user_id = $1", user_id);

        txn.commit();

        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error creating order: " << e.what() << std::endl;
        return false;
    }
}

bool Database::addReview(int product_id,
                         const std::string& user_name,
                         const std::string& review_date,
                         const std::string& review_text,
                         const std::string& selected_size,
                         int rating,
                         const std::vector<std::string>& image_urls) {
    try {
        pqxx::work txn(*conn_);

        pqxx::result r = txn.exec_params(
            "INSERT INTO reviews (product_id, user_name, review_date, review_text, selected_size, rating) "
            "VALUES ($1, $2, $3, $4, $5, $6) RETURNING id",
            product_id, user_name, review_date, review_text, selected_size, rating
        );

        int review_id = r[0][0].as<int>();

        for (const auto& url : image_urls) {
            txn.exec_params(
                "INSERT INTO review_images (review_id, image_url) VALUES ($1, $2)",
                review_id, url
            );
        }

        txn.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error adding review: " << e.what() << std::endl;
        return false;
    }
}

