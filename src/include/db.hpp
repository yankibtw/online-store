#ifndef DB_HPP
#define DB_HPP

#include <pqxx/pqxx>
#include <string>
#include <optional>
#include "..\..\include\crow\include\crow_all.h"

struct Product {
    int id;
    std::string name;
    std::string brand;
    std::string image_url;
    double price;
    double discount_price;
    std::string description;
    std::string sku;

    crow::json::wvalue toJson() const {
        crow::json::wvalue json;
        json["id"] = id;
        json["name"] = name;
        json["brand"] = brand;
        json["image_url"] = image_url;
        json["price"] = price;
        json["discount_price"] = discount_price;
        json["description"] = description;
        json["sku"] = sku;
        return json;
    }
};

struct CartProduct {
    int cart_item_id;
    int product_id;
    std::string name;
    double price;
    std::optional<double> discount_price;
    std::string image_url;
    int quantity;
    std::string size;
    std::string sku;
};

class Database {
public:
    Database(const std::string& db_name, const std::string& db_user,
             const std::string& db_password, const std::string& db_host, int db_port);
    ~Database();

    bool connect();
    bool registerUser(const std::string& firstName, const std::string& lastName,
                      const std::string& email, const std::string& phone,
                      const std::string& password);
    std::optional<std::string> authenticateUser(const std::string& email, const std::string& password, bool& userNotFound);
    std::string createSession(const std::string& user_id);
    bool checkSession(const std::string& session_id);
    bool deleteSession(const std::string& session_id);
    bool isEmailAlreadyRegistered(const std::string& email);
    std::vector<Product> getProducts(int limit = 20);
    Product getProductById(int id);
    std::vector<crow::json::wvalue> getReviewsByProduct(int productId);
    bool addToFavorites(const std::string& session_id, int product_id);
    std::vector<Product> getFavoritesBySessionId(const std::string& session_id);
    bool removeFromFavorites(const std::string& session_id, int product_id);
    void addProductToCart(const std::string& session_id, int product_id, const std::string& size);
    std::vector<CartProduct> getCartBySessionId(const std::string& session_id);
    bool removeFromCart(const std::string& session_id, int product_id);
    void updateCartItemQuantity(int cart_item_id, int quantity);
private:
    std::string generateSessionId();
    std::string hashPassword(const std::string& password);

    std::string db_name_, db_user_, db_password_, db_host_;
    int db_port_;
    pqxx::connection* conn_;
};

#endif
