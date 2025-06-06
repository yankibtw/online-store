cmake_minimum_required(VERSION 3.10)
project(CrowServer)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

add_definitions(-DCROW_USE_STANDALONE_ASIO)

include_directories(${CMAKE_SOURCE_DIR}/include/crow)

find_package(libpqxx CONFIG REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(unofficial-sodium CONFIG REQUIRED)
find_package(CURL REQUIRED)

add_executable(server   src/main.cpp 
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
                        src/controllers/page_controller.cpp)
                        

target_link_libraries(server PRIVATE libpqxx::pqxx)
target_link_libraries(server PRIVATE unofficial-sodium::sodium)
target_link_libraries(server PRIVATE CURL::libcurl)
