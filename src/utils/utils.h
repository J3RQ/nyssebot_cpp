#ifndef NYSSEBOT_UTILS_H
#define NYSSEBOT_UTILS_H
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <iostream>
#include <ctime>
#include <regex>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <fmt/format.h>
#include <dpp/message.h>

class Config
{
public:
    std::string token;
    std::string mapBoxApikey;
    void getConfig();
};

std::vector<std::string> splitString(std::string inputString, char splitChar);
std::map<std::string, int> getTime(std::time_t timestamp);
bool stringInVector (std::string key, std::vector<std::string> checkVec);
time_t paramsToTimestamp(std::map<std::string, std::string> parameters, bool hourEntered, bool minuteEntered, bool dateEntered);
std::string cleanTime(std::time_t);

class Eventcache {
public:
    std::map<dpp::snowflake, std::map<std::string, std::string>> eventMap; 
    std::map<std::string, std::string> getEvent(dpp::snowflake messageId);
    void setEvent(dpp::snowflake messageId, std::map<std::string, std::string> event);
    void removeEvent(dpp::snowflake messageId);
};

#endif //NYSSEBOT_UTILS_H
