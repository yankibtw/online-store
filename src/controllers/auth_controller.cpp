#include "../include/controllers/auth_controller.hpp"
#include "../include/controllers/cookie_utils.hpp"
#include <sstream>

void AuthController::setup(crow::SimpleApp& app, UserManager& db) {
    CROW_ROUTE(app, "/register").methods("POST"_method)([&db](const crow::request& req) {
        auto x = crow::json::load(req.body);
        if (!x) {
            return crow::response(400, crow::json::wvalue{{"error", "Invalid JSON"}}.dump());
        }

        std::string firstName = x["firstName"].s();
        std::string lastName = x["lastName"].s();
        std::string email = x["email"].s();
        std::string phone = x["phone"].s();
        std::string password = x["password"].s();

        if (firstName.empty() || lastName.empty() || password.empty()) {
            return crow::response(400, crow::json::wvalue{{"error", "First name, last name, and password are required"}}.dump());
        }

        if (db.isEmailAlreadyRegistered(email)) {
            return crow::response(400, crow::json::wvalue{{"error", "Email is already registered"}}.dump());
        }

        if (db.registerUser(firstName, lastName, email, phone, password)) {
            bool userNotFound = false;
            auto userIdOpt = db.authenticateUser(email, password, userNotFound);

            if (userIdOpt) {
                std::string sessionId = db.createSession(*userIdOpt);
                if (!sessionId.empty()) {
                    crow::json::wvalue response;
                    response["message"] = "Registration and login successful";
                    response["sessionId"] = sessionId;

                    crow::response res(200);
                    res.set_header("Set-Cookie", "session_id=" + sessionId + "; Path=/; HttpOnly; SameSite=Lax");
                    res.write(response.dump());
                    res.set_header("Content-Type", "application/json");

                    return res;
                } else {
                    return crow::response(500, crow::json::wvalue{{"error", "Failed to create session"}}.dump());
                }
            } else {
                return crow::response(500, crow::json::wvalue{{"error", "Authentication failed"}}.dump());
            }
        } else {
            return crow::response(500, crow::json::wvalue{{"error", "Registration failed"}}.dump());
        }
    });

    CROW_ROUTE(app, "/login").methods("POST"_method)([&db](const crow::request& req) {
        auto x = crow::json::load(req.body);
        if (!x) return crow::response(400, R"({"error": "Invalid JSON"})");

        bool userNotFound = false;
        auto userIdOpt = db.authenticateUser(x["email"].s(), x["password"].s(), userNotFound);

        if (userIdOpt) {
            std::string sessionId = db.createSession(*userIdOpt);
            crow::response res(200);
            res.set_header("Set-Cookie", "session_id=" + sessionId + "; Path=/; HttpOnly");
            res.write(R"({"message": "Login successful", "sessionId": ")" + sessionId + "\"}");
            return res;
        }

        return userNotFound
               ? crow::response(404, R"({"error": "User not found"})")
               : crow::response(401, R"({"error": "Wrong password"})");
    });

    CROW_ROUTE(app, "/logout").methods("POST"_method)([&db](const crow::request& req) {
        std::string session_id = extractSessionId(req.get_header_value("Cookie"));
        if (!session_id.empty()) {
            db.deleteSession(session_id);
            return crow::response(200, R"({"message": "Logged out"})");
        }
        return crow::response(400, R"({"error": "No session"})");
    });
}