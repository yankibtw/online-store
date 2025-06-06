#pragma once
#ifndef DB_HPP
#define DB_HPP

#include <pqxx/pqxx>
#include <string>
#include <iostream>
#include <random>
#include "..\..\include\crow\include\crow_all.h"

struct Image {
    std::string url;
    bool is_main;
};

struct Product {
    int id;
    std::string name;
    std::string brand;
    std::string image_url;
    std::vector<Image> images; 
    double price;
    double discount_price;
    std::string description;
    std::string sku;
    std::string category;
    int stock_quantity = 0;
    std::string parent_category;

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
        json["category"] = category;
        json["stock_quantity"] = stock_quantity;
        json["parent_category"] = parent_category;

        crow::json::wvalue images_json = crow::json::wvalue::list();
        for (size_t i = 0; i < images.size(); ++i) {
            images_json[i]["url"] = images[i].url;
            images_json[i]["is_main"] = images[i].is_main;
        }
        json["images"] = std::move(images_json);

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

struct OrderItem {
    int product_id;
    std::string name;
    std::string size;
    int quantity;
    double price;
    std::optional<double> discount_price;
    std::string image_url;
    std::string sku;
};

struct Order {
    int order_id;
    std::string payment_method;
    std::string status;
    std::string address;
    std::string created_at;
    std::vector<OrderItem> items;
};

class Database {
public:
    Database(const std::string& db_name, const std::string& db_user,
             const std::string& db_password, const std::string& db_host, int db_port);
    ~Database();

    bool connect();
    std::string generateSessionId();
    pqxx::connection* getConnection();
    std::string extractSessionId(const std::string& cookieHeader);

private:
    std::string db_name_;
    std::string db_user_;
    std::string db_password_;
    std::string db_host_;
    int db_port_;
    pqxx::connection* conn_;
};

#endif 