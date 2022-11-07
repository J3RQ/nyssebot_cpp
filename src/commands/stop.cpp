#include "stop.h"
#include "utils.h"

using json = nlohmann::json;

std::map<std::string, int> getTime (std::time_t timestamp) {
    std::map<std::string, int> timeMap;
    auto tm = *std::localtime(&timestamp);
    std::ostringstream day, month, year, hour, minute;
    day << std::put_time(&tm, "%d"), month << std::put_time(&tm, "%m"), year << std::put_time(&tm, "%Y");
    hour << std::put_time(&tm, "%H"), minute << std::put_time(&tm, "%M");
    timeMap["day"] = std::stoi(day.str()), timeMap["month"] = std::stoi(month.str()), timeMap["year"] = std::stoi(year.str());
    timeMap["hour"] = std::stoi(hour.str()), timeMap["minute"] = std::stoi(minute.str());
    return timeMap; 
}

bool stringInVector (std::string key, std::vector<std::string> checkVec) {
    return (std::find(checkVec.begin(), checkVec.end(), key) == checkVec.end());
}

std::string stopSearch(std::map<std::string, std::string> stopParams) {
     
    json stopPayload;
    int departureAmount = (stopParams.count("line") == 0) ? 16 : 24;

    std::time_t epochTimestamp = std::time(nullptr);     

    bool hourEntered = stopParams.count("hour") != 0;
    bool minuteEntered = stopParams.count("minute") != 0;
    bool dateEntered = stopParams.count("date") != 0;

    if (hourEntered || minuteEntered || dateEntered) {
        std::map<std::string, int> searchTime = getTime(epochTimestamp);
        
        if (hourEntered) {
            searchTime["hour"] = std::stoi(stopParams["hour"]);
        } 
        if (minuteEntered) {
            searchTime["minute"] = std::stoi(stopParams["minute"]);
        }
        
        if (dateEntered) {
            std::vector<std::string> date = splitString(stopParams["date"], '.');
            if (date.size() == 2) {
                searchTime["day"] = std::stoi(date[0]), searchTime["month"] = std::stoi(date[1]);
            } else if(date.size() == 3) {
                searchTime["day"] = std::stoi(date[0]), searchTime["month"] = std::stoi(date[1]), searchTime["year"] = std::stoi(date[2]);
            } else {
                return std::string("```Invalid date.```");
            }
        } 
    
        struct tm t = {0};
        t.tm_year = searchTime["year"] - 1900;
        t.tm_mon = searchTime["month"] - 1;
        t.tm_mday = searchTime["day"];
        t.tm_hour = searchTime["hour"];
        t.tm_min =  searchTime["minute"];
        std::time_t timeSinceEpoch = mktime(&t);

        epochTimestamp = timeSinceEpoch;
    }
            
    stopPayload["query"] = fmt::format("query {{\n"
    "stop(id: \"tampere:{}\") {{\n"
        "name\n"
        "stoptimesWithoutPatterns(numberOfDepartures: {}, startTime: {}, omitNonPickups: true, timeRange: 10800) {{\n"
            "realtimeArrival\n"
            "arrivalDelay\n"
            "scheduledDeparture\n"
            "realtimeDeparture\n"
            "departureDelay\n"
            "realtime\n"
            "headsign\n"
            "timepoint\n"
            "pickupType\n"
            "trip {{\n"
                "route {{\n"
                    "shortName\n"
                    "longName\n"
                    "}}\n"
                "}}\n"
            "}}\n"      
        "}}\n"   
    "}}\n", stopParams["query"], departureAmount, epochTimestamp);


    cpr::Response r = cpr::Post(cpr::Url{"https://api.digitransit.fi/routing/v1/routers/waltti/index/graphql"},
    cpr::Body{stopPayload.dump()},
    cpr::Header{{"Content-Type", "application/json"}});

    json responseJson = json::parse(r.text)["data"]["stop"];
    std::string message;
    if (responseJson.is_null()) {
        message.append("```That stop doesn't exist.```");
        return message;
    }

    if (responseJson["stoptimesWithoutPatterns"].size() > 0)
    {
        message.append(fmt::format("**__{}:__**", responseJson["name"].get<std::string>()));

        std::vector<std::string> lineVec;
        if (stopParams.count("line") != 0) {
            std::regex regFormat("[a-zA-Z0-9]+");
            for(std::sregex_iterator i = std::sregex_iterator(stopParams["line"].begin(), stopParams["line"].end(), regFormat); i != std::sregex_iterator(); ++i ) {
                std::smatch m = *i;
                lineVec.push_back(m.str());
            }
        }

        for (json depDict : responseJson["stoptimesWithoutPatterns"]) {
            if ((stopParams.count("line") != 0) && (stringInVector(depDict["trip"]["route"]["shortName"], lineVec))) {
                continue;
            }
            int timestamp = depDict["realtimeDeparture"].get<int>();
            timestamp = (timestamp > 86400) ? timestamp - 86400 : timestamp;
            std::string parsedTime = fmt::format("{:02}:{:02}", timestamp / (60*60), (timestamp % (60*60)) / 60);
            message.append(fmt::format("```Linja: {}\n\tMääränpää: {}\n\tAika: {}```", depDict["trip"]["route"]["shortName"].get<std::string>(), depDict["headsign"].get<std::string>(), parsedTime));
            }
        } else {
            message.append("```No departures in the next three hours.```");
        }
        return message;
    }
    
dpp::message stopSelect(dpp::snowflake channel, std::map<std::string, std::string> stopParams) {
    json stopPayload;
    dpp::message message(channel, "");

    stopPayload["query"] = fmt::format("query {{\n"
        "stops(name: \"{}\") {{\n"
            "name\n"
            "gtfsId\n"
            "code\n"
            "}}\n"     
        "}}\"\n"
    ")", stopParams["query"]);

    cpr::Response r = cpr::Post(cpr::Url{"https://api.digitransit.fi/routing/v1/routers/waltti/index/graphql"},
    cpr::Body{stopPayload.dump()},
    cpr::Header{{"Content-Type", "application/json"}});

    json responseJson = json::parse(r.text)["data"]["stops"];

    if (responseJson.size() > 1 && responseJson.dump().find("tampere") != std::string::npos)
    {   
        std::string messageBuilder = "__**Choose stop**__:\n";
        int index = 0;
        for (json stopDict : responseJson) {
            if (stopDict["gtfsId"].dump().find("tampere") != std::string::npos) {  
                messageBuilder.append(fmt::format("```{} (ID: {})```", stopDict["name"].get<std::string>(), stopDict["code"].get<std::string>()));
                index += 1;   
            }
        };
        message.set_content(messageBuilder);
        return message;
    } else if (responseJson.size() > 0 && responseJson[0]["gtfsId"].dump().find("tampere") != std::string::npos) {
        stopParams["query"] = responseJson[0]["code"];
        return dpp::message(channel, stopSearch(stopParams));
    } else {
        message.set_content("```No stops found.```");
    }
    return message;
}
