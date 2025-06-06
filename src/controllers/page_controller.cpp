#include "../include/controllers/page_controller.hpp"

void PageController::setup(crow::SimpleApp& app) {
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

    CROW_ROUTE(app, "/order")([]() {
        return crow::mustache::load("order.html").render();
    });

    CROW_ROUTE(app, "/favourite")([]() {
        return crow::mustache::load("favourite.html").render();
    });

    CROW_ROUTE(app, "/politika")([]() {
        return crow::mustache::load("politika.html").render();
    });

    CROW_ROUTE(app, "/swap")([]() {
        return crow::mustache::load("swap.html").render();
    });

    CROW_ROUTE(app, "/user_order")([]() {
        return crow::mustache::load("user_orders.html").render();
    });
}