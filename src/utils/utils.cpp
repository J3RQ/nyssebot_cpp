#include "utils.h"

void Config::getConfig() {   
    std::ifstream ifs(".env");
    if (ifs.good())
    {
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    std::vector<std::string> configs = splitString(content, '\n');

    std::map<std::string, std::string> configmap;
    std::string possibleParams[2] = {"TOKEN", "MB_APIKEY"};

    for (std::size_t i = 0; i < configs.size(); i++) {
        std::vector<std::string> splitParam = splitString(configs[i], '=');
        if (std::find(std::begin(possibleParams), std::end(possibleParams), splitParam[0]) != std::end(possibleParams))
        {
            configmap.insert(std::pair<std::string, std::string>(splitParam[0], splitParam[1]));
        }
    }
    token = configmap.at("TOKEN");
    mapBoxApikey = configmap.at("MB_APIKEY");
    } else {
        std::cout << "Invalid .env!";
    }
}

std::vector<std::string> splitString(std::string inputString, char splitChar) {
    std::stringstream wholeString(inputString);
    std::string stringPart;
    std::vector<std::string> paramlist;

    while (std::getline(wholeString, stringPart, splitChar)) {
        paramlist.push_back(stringPart);
    }
    return paramlist;
}

std::map<std::string, int> getTime (std::time_t timestamp) {
    std::map<std::string, int> timeMap;
    auto tm = *std::localtime(&timestamp);
    std::ostringstream day, month, year, hour, minute;
    day << std::put_time(&tm, "%d"), month << std::put_time(&tm, "%m"), year << std::put_time(&tm, "%Y");
    hour << std::put_time(&tm, "%H"), minute << std::put_time(&tm, "%M");
    timeMap["day"] = std::stoi(day.str());
    timeMap["month"] = std::stoi(month.str()); 
    timeMap["year"] = std::stoi(year.str());
    timeMap["hour"] = std::stoi(hour.str()); 
    timeMap["minute"] = std::stoi(minute.str());
    return timeMap; 
}

bool stringInVector (std::string key, std::vector<std::string> checkVec) {
    return (std::find(checkVec.begin(), checkVec.end(), key) == checkVec.end());
}


std::map<std::string, std::string> Eventcache::getEvent(dpp::snowflake messageId) {
    return eventMap[messageId];
}

void Eventcache::setEvent(dpp::snowflake messageId, std::map<std::string, std::string> event) {
    eventMap[messageId] = event;
}

void Eventcache::removeEvent(dpp::snowflake messageId) {
    eventMap.erase(messageId);
}
