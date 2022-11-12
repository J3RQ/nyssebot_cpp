#ifndef NYSSEBOT_STOP_H
#define NYSSEBOT_STOP_H
#include <dpp/nlohmann/json.hpp>
#include <dpp/dpp.h>
#include <cpr/cpr.h>
#include <fmt/format.h>
#include <string>
#include <iostream>
#include <ctime>
#include <regex>
#include <algorithm>
#include <iomanip>
#include <sstream>

dpp::message stopMain(dpp::snowflake channel, std::map<std::string, std::string> stopParams);
dpp::message stopSearch(dpp::snowflake channel, std::map<std::string, std::string> stopParams);
json stopRequest(std::string query);
dpp::message stopSelect(dpp::snowflake channel, std::map<std::string, std::string> stopParams, std::string selectorId);

#endif //NYSSEBOT_STOP_H

