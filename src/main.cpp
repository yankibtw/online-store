#define CROW_STATIC_DIRECTORY "C:/test/static"
#include "include/controllers/auth_controller.hpp"
#include "include/controllers/product_controller.hpp"
#include "include/controllers/cart_controller.hpp"
#include "include/controllers/wishlist_controller.hpp"
#include "include/controllers/order_controller.hpp"
#include "include/controllers/page_controller.hpp"

int main() {
    crow::SimpleApp app;
    crow::mustache::set_global_base("C:/test/templates");

    Database db("online-store", "postgres", "1234", "localhost", 5432);
    if (db.connect()) {
        UserManager userManager(db);
        CartManager cartManager(db);
        OrderManager orderManager(db);
        ProductManager productManager(db);
        WishlistManager wishlistManager(db);

        AuthController::setup(app, userManager);
        ProductController::setup(app, productManager);
        CartController::setup(app, cartManager, userManager);
        WishlistController::setup(app, wishlistManager, userManager);
        OrderController::setup(app, orderManager, userManager);

        PageController::setup(app);

        std::cout << "Connection successful!" << std::endl;
    } else {
        std::cerr << "Failed to connect to the database!" << std::endl;
    }
    
    app.port(18080).multithreaded().run();

    return 0;
}
