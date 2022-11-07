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


std::string stopSearch(std::map<std::string, std::string> stopParams);
dpp::message stopSelect(dpp::snowflake channel, std::map<std::string, std::string> stopParams);

#endif //NYSSEBOT_STOP_H

