cmake_minimum_required(VERSION 3.10)
project(CrowServer)

set(CMAKE_CXX_STANDARD 17)

# Указываем путь к toolchain vcpkg
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

# Определяем использование standalone Asio для Crow
add_definitions(-DCROW_USE_STANDALONE_ASIO)

# Подключаем заголовки Crow (если не через vcpkg)
include_directories(${CMAKE_SOURCE_DIR}/include/crow)

# Только то, что установлено через vcpkg
find_package(libpqxx CONFIG REQUIRED)

add_executable(server src/main.cpp)

target_link_libraries(server PRIVATE libpqxx::pqxx)
