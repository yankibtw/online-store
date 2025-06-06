#include "../include/controllers/auth_controller.hpp"
#include <sstream>

namespace {
    std::string extractSessionId(const std::string& cookieHeader) {
    std::istringstream ss(cookieHeader);
    std::string token;
    while (std::getline(ss, token, ';')) {
        auto pos = token.find('=');
        if (pos != std::string::npos) {
            std::string key = token.substr(0, pos);
            std::string value = token.substr(pos + 1);
            while (!key.empty() && key.front() == ' ') key.erase(0, 1);
            if (key == "session_id") return value;
        }
    }
    return "";
}
}

void AuthController::setup(crow::SimpleApp& app, UserManager& db) {
    CROW_ROUTE(app, "/register").methods("POST"_method)([&db](const crow::request& req) {
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