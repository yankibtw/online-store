#ifndef APP_HPP
#define APP_HPP

#include "db.hpp"
#include "..\..\include\crow\include\crow_all.h"

void setupRoutes(crow::SimpleApp& app, Database& db);

#endif
