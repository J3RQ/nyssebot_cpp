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

class Config
{
public:
    std::string token;
    std::string mapBoxApikey;
    void getConfig();
};

std::vector<std::string> splitString(std::string inputString, char splitChar);
std::map<std::string, int> getTime (std::time_t timestamp);
bool stringInVector (std::string key, std::vector<std::string> checkVec);

#endif //NYSSEBOT_UTILS_H