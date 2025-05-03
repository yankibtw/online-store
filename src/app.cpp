#include "include/app.hpp"
#include "..\include\crow\include\crow_all.h"

std::string extractSessionId(const std::string& cookieHeader) {
    std::istringstream ss(cookieHeader);
    std::string token;
    while (std::getline(ss, token, ';')) {
        auto pos = token.find('=');
        if (pos != std::string::npos) {
            std::string key = token.substr(0, pos);
            std::string value = token.substr(pos + 1);
            while (!key.empty() && key.front() == ' ') key.erase(0, 1);
            if (key == "session_id") {
                return value;
            }
        }
    }
    return "";
}

void setupRoutes(crow::SimpleApp& app, Database& db) {
    CROW_ROUTE(app, "/")([]() {
        return crow::mustache::load("main.html").render();
    });

    CROW_ROUTE(app, "/catalog")([]() {
        return crow::mustache::load("catalog.html").render();
    });

    CROW_ROUTE(app, "/register").methods("POST"_method)
    ([&db](const crow::request& req) {
        // Загружаем JSON из тела запроса
        auto x = crow::json::load(req.body);
        if (!x) {
            return crow::response(400, crow::json::wvalue{{"error", "Invalid JSON"}}.dump());
        }
    
        // Извлекаем поля из JSON
        std::string firstName = x["firstName"].s();
        std::string lastName = x["lastName"].s();
        std::string email = x["email"].s();
        std::string phone = x["phone"].s();
        std::string password = x["password"].s();
    
        // Проверяем, чтобы обязательные поля не были пустыми
        if (firstName.empty() || lastName.empty() || password.empty()) {
            return crow::response(400, crow::json::wvalue{{"error", "First name, last name and password are required"}}.dump());
        }
    
        // Регистрация пользователя в базе данных
        if (db.registerUser(firstName, lastName, email, phone, password)) {
            // Аутентифицируем пользователя сразу после регистрации
            auto userIdOpt = db.authenticateUser(email, password);
            if (userIdOpt) {
                // Создаем сессию для аутентифицированного пользователя
                std::string sessionId = db.createSession(*userIdOpt);
                if (!sessionId.empty()) {
                    // Формируем JSON-ответ
                    crow::json::wvalue response;
                    response["message"] = "Registration and login successful";
                    response["sessionId"] = sessionId;
    
                    // Устанавливаем cookie для сессии
                    crow::response res(200);
                    res.set_header("Set-Cookie", "session_id=" + sessionId + "; Path=/; HttpOnly");
                    res.write(response.dump());  // Сериализуем JSON в строку
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
    
    CROW_ROUTE(app, "/login").methods("POST"_method)
    ([&db](const crow::request& req) {
        auto x = crow::json::load(req.body);
        if (!x) {
            return crow::response(400, crow::json::wvalue{{"error", "Invalid JSON"}});
        }
    
        auto email = x["email"].s();
        auto password = x["password"].s();
    
        auto userIdOpt = db.authenticateUser(email, password);
        if (userIdOpt.has_value()) {
            std::string sessionId = db.createSession(*userIdOpt);
            if (!sessionId.empty()) {
                crow::response res(200);
                res.set_header("Set-Cookie", "session_id=" + sessionId + "; Path=/; HttpOnly");
                res.write(crow::json::wvalue{{"message", "Login successful"}, {"sessionId", sessionId}}.dump());
                return res;
            }
        }
        return crow::response(401, crow::json::wvalue{{"error", "Invalid credentials"}});
    });
    
    
    
    CROW_ROUTE(app, "/logout").methods("POST"_method)
    ([&db](const crow::request& req) {
        std::string raw_cookie = req.get_header_value("Cookie");
        std::string session_id = extractSessionId(raw_cookie); 
        if (!session_id.empty()) {
            db.deleteSession(session_id);
            return crow::response(200, crow::json::wvalue{{"message", "Logged out"}});
        }
        return crow::response(400, crow::json::wvalue{{"error", "No session"}});
    });
    
    
    CROW_ROUTE(app, "/profile").methods("GET"_method)
    ([&db](const crow::request& req) {
        std::string session_id = extractSessionId(req.get_header_value("Cookie"));
        if (!session_id.empty() && db.checkSession(session_id)) {
            return crow::response(200, "Welcome to your profile");
        }
        return crow::response(401, "Unauthorized");        
    });
    
    
}