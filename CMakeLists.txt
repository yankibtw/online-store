cmake_minimum_required(VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

project(CrowServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include/crow)

find_package(libpqxx CONFIG REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(unofficial-sodium CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)

add_executable(server
    src/main.cpp 
    src/wishlist_manager.cpp
    src/user_manager.cpp
    src/product_manager.cpp
    src/order_manager.cpp
    src/dbb.cpp
    src/cart_manager.cpp
    src/controllers/auth_controller.cpp
    src/controllers/cart_controller.cpp
    src/controllers/cookie_utils.cpp
    src/controllers/order_controller.cpp
    src/controllers/product_controller.cpp
    src/controllers/wishlist_controller.cpp
    src/controllers/page_controller.cpp
)

target_link_libraries(server PRIVATE
    libpqxx::pqxx
    unofficial-sodium::sodium
    CURL::libcurl
    nlohmann_json::nlohmann_json
)

target_compile_definitions(server PRIVATE CROW_USE_STANDALONE_ASIO)

enable_testing()
find_package(GTest CONFIG REQUIRED)

add_executable(tests
    tests/test_dbb.cpp
    tests/test_user_manager.cpp
    tests/test_cart_manager.cpp
    tests/test_product_manager.cpp
    tests/test_order_manager.cpp
    tests/test_wishlist_manager.cpp
    src/dbb.cpp
    src/user_manager.cpp
    src/cart_manager.cpp
    src/product_manager.cpp
    src/order_manager.cpp
    src/wishlist_manager.cpp
)

target_link_libraries(tests PRIVATE Threads::Threads)

target_link_libraries(tests PRIVATE
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main
    libpqxx::pqxx
    unofficial-sodium::sodium
)

add_test(NAME AllTestsInTests COMMAND tests)
