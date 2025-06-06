#pragma once
#ifndef ORDER_MANAGER_HPP
#define ORDER_MANAGER_HPP

#include "dbb.hpp"
#include <string>
#include <vector>

class OrderManager {
public:
    explicit OrderManager(Database& db);

    bool createOrder(const std::string& session_id,
                     const std::string& payment_method,
                     const std::string& address);

    bool addReview(int product_id,
                   const std::string& user_name,
                   const std::string& review_date,
                   const std::string& review_text,
                   const std::string& selected_size,
                   int rating,
                   const std::vector<std::string>& image_urls);

    std::vector<Order> getOrderHistory(const std::string& session_id);

private:
    Database& db_;
};

#endif 