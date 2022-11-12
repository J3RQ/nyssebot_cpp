#ifndef NYSSEBOT_MAP_H
#define NYSSEBOT_MAP_H

#include <dpp/nlohmann/json.hpp>
#include <dpp/message.h>
#include <cpr/cpr.h>
#include <fmt/format.h>
#include <string>
#include <fstream>

dpp::message getMap(dpp::snowflake channel, std::map<std::string, std::string> parameters);

#endif //NYSSEBOT_MAP_H