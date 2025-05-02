#define CROW_STATIC_DIRECTORY "C:/test/static"
#include "..\include\crow\include\crow_all.h"

int main() {
    crow::SimpleApp app;

    crow::mustache::set_global_base("C:/test/templates");

    CROW_ROUTE(app, "/")([]() {
        return crow::mustache::load("main.html").render();
    });

    CROW_ROUTE(app, "/catalog")([]() {
        return crow::mustache::load("catalog.html").render();
    });

    app.port(18080).multithreaded().run();
}