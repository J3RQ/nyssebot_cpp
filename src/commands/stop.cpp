#include "stop.h"
#include "utils.h"

using json = nlohmann::json;

std::map<std::string, int> getTime (std::time_t timestamp) {
    std::map<std::string, int> timeMap;
    auto tm = *std::gmtime(&timestamp);
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
     
    bool isId = true;
    for(char& c : stopParams["query"]) {
        if (isdigit(c) == false) {
            isId = false;
        }
    }

    if (isId) {
        json stopPayload;
        int departureAmount = (stopParams.count("line") == 0) ? 16 : 24;

        std::time_t timeNow = std::time(nullptr);     

        bool hourEntered = stopParams.count("hour") != 0;
        bool minuteEntered = stopParams.count("minute") != 0;
        bool dateEntered = stopParams.count("date") != 0;

        if (hourEntered || minuteEntered || dateEntered) {
            std::map<std::string, int> curTime = getTime(timeNow);
            if (hourEntered && minuteEntered) {
                curTime["hour"] = std::stoi(stopParams["hour"]), curTime["minute"] = std::stoi(stopParams["minute"]);
            } else if (dateEntered && hourEntered && minuteEntered) {
                curTime["hour"] = std::stoi(stopParams["hour"]), curTime["minute"] = std::stoi(stopParams["minute"]);
                std::vector<std::string> date = splitString(stopParams["date"], '.');
                if (date.size() == 2) {
                    curTime["day"] = std::stoi(date[0]), curTime["month"] = std::stoi(date[1]);
                } else if(date.size() == 3) {
                    curTime["day"] = std::stoi(date[0]), curTime["month"] = std::stoi(date[1]), curTime["year"] = std::stoi(date[2]);
                } else {
                    return std::string("```Invalid date.```");
                }
            } else if (hourEntered && !minuteEntered) {
                curTime["hour"] = std::stoi(stopParams["hour"]);
            } else if (!hourEntered && minuteEntered) {
                curTime["minute"] = std::stoi(stopParams["minute"]);
            }

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
        "}}\n", stopParams["query"], departureAmount, timeNow);


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
    } else {
        json stopPayload;
        
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
        std::string message;

        if (responseJson.size() > 1 && responseJson.dump().find("tampere") != std::string::npos)
        {   
            message.append("**__Choose stop:__**");

            int index = 1;
            for (json stopDict : responseJson) {
                if (stopDict["gtfsId"].dump().find("tampere") != std::string::npos)
                {
                    message.append(fmt::format("```{}: {} (ID: {})```", index, stopDict["name"].get<std::string>(), stopDict["code"].get<std::string>()));
                    index += 1;   
                }
            }
        } else if (responseJson.size() > 0 && responseJson[0]["gtfsId"].dump().find("tampere") != std::string::npos) {
            stopParams["query"] = responseJson[0]["code"];
            return stopSearch(stopParams);
        } else {
            message.append("```No stops found.```");
        }
        return message;
    }
}
