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

    // Регистрация пользователя
    CROW_ROUTE(app, "/register").methods("POST"_method)
    ([&db](const crow::request& req) {
        auto x = crow::json::load(req.body);
        if (!x) {
            return crow::response(400, "Invalid JSON");
        }

        std::string firstName = x["firstName"].s();
        std::string lastName = x["lastName"].s();
        std::string email = x["email"].s();
        std::string phone = x["phone"].s();
        std::string password = x["password"].s();

        if (firstName.empty() || lastName.empty() || password.empty()) {
            return crow::response(400, "First name, last name and password are required");
        }

        if (db.registerUser(firstName, lastName, email, phone, password)) {
            return crow::response(200, "Registration successful");
        } else {
            return crow::response(500, "Registration failed");
        }
    });

    // Аутентификация без сессий
    CROW_ROUTE(app, "/login").methods("POST"_method)
    ([&db](const crow::request& req) {
        auto x = crow::json::load(req.body);
        if (!x) {
            return crow::response(400, "Invalid JSON");
        }

        std::string email = x["email"].s();
        std::string password = x["password"].s();

        if (email.empty() || password.empty()) {
            return crow::response(400, "Email and password are required");
        }

        if (db.authenticateUser(email, password)) {
            return crow::response(200, "Login successful");
        } else {
            return crow::response(401, "Invalid credentials");
        }
    });

    app.port(18080).multithreaded().run();
}
