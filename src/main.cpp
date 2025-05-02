#define CROW_STATIC_DIRECTORY "C:/test/static"
#include "..\include\crow\include\crow_all.h"
#include "db.cpp"

int main() {
    crow::SimpleApp app;
    crow::mustache::set_global_base("C:/test/templates");

    Database db("online-store", "postgres", "1234", "localhost", 5432);

    if (db.connect()) {
        std::cout << "Connection successful!" << std::endl;
    } else {
        std::cerr << "Failed to connect to the database!" << std::endl;
    }

    CROW_ROUTE(app, "/")([]() {
        return crow::mustache::load("main.html").render();
    });

    CROW_ROUTE(app, "/catalog")([]() {
        return crow::mustache::load("catalog.html").render();
    });

    app.port(18080).multithreaded().run();
}