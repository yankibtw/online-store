cmake_minimum_required(VERSION 3.10)
project(CrowServer)
add_definitions(-DCROW_USE_STANDALONE_ASIO)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

set(CMAKE_CXX_STANDARD 17)

# Указываем путь к заголовочным файлам Crow
include_directories(${CMAKE_SOURCE_DIR}/include/crow)
include_directories("C:/vcpkg/installed/x64-windows/include")

add_executable(server src/main.cpp)
