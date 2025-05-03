#define CROW_STATIC_DIRECTORY "C:/test/static"
#include "app.cpp"
#include "db.cpp"

int main() {
    crow::SimpleApp app;
    crow::mustache::set_global_base("C:/test/templates");

    Database db("online-store", "postgres", "1234", "localhost", 5432);
    if (db.connect()) {
        setupRoutes(app, db);
        std::cout << "Connection successful!" << std::endl;
    } else {
        std::cerr << "Failed to connect to the database!" << std::endl;
    }

    app.port(18080).multithreaded().run();

    return 0;
}
