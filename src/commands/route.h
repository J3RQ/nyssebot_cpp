#ifndef NYSSEBOT_ROUTE_H
#define NYSSEBOT_ROUTE_H

#include <dpp/nlohmann/json.hpp>
#include <dpp/message.h>
#include <cpr/cpr.h>
#include <fmt/format.h>
#include <string>

dpp::message route(dpp::snowflake channel, std::map<std::string, std::string> stopParams);


#endif //NYSSEBOT_ROUTE_H
