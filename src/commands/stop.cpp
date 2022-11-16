#include "stop.h"
#include "utils.h"

using json = nlohmann::json;

dpp::message stopMain(dpp::snowflake channel, std::map<std::string, std::string> stopParams) {
    bool isId = true;
    for(char& c : stopParams["query"]) {
        if (isdigit(c) == false) {
            isId = false;
        }
    }
    if (isId) {
        return stopSearch(channel, stopParams);
    } else {
        return stopSelect(channel, stopParams, std::string("stopSelector"));
    }
}

dpp::message stopSearch(dpp::snowflake channel, std::map<std::string, std::string> stopParams) {
    json stopPayload;

    std::time_t epochTimestamp = std::time(nullptr);     

    bool hourEntered = stopParams.count("hour") != 0;
    bool minuteEntered = stopParams.count("minute") != 0;
    bool dateEntered = stopParams.count("date") != 0;

    int departureAmount = (stopParams.count("line") == 0) ? 16 : 24;
    
    if (hourEntered || minuteEntered || dateEntered) {
        epochTimestamp = paramsToTimestamp(stopParams, hourEntered, minuteEntered, dateEntered);
    }

    if (epochTimestamp == time_t(0)) return dpp::message(channel, "```Invalid date expression.```");
            
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
        return dpp::message(channel, message);
    }

    if (responseJson["stoptimesWithoutPatterns"].size() > 0) {
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
            message.append(fmt::format("```{} | {} {}```", parsedTime, depDict["trip"]["route"]["shortName"].get<std::string>(), depDict["headsign"].get<std::string>()));
            }
        } else {
            message.append("```No departures in the next three hours.```");
        }
        return dpp::message(channel, message);
    }

json stopRequest(std::string query) {
    json stopPayload;

    stopPayload["query"] = fmt::format("query {{\n"
        "stops(name: \"{}\") {{\n"
            "name\n"
            "gtfsId\n"
            "code\n"
            "lat\n"
            "lon\n"
            "}}\n"     
        "}}\"\n"
    ")", query);

    cpr::Response r = cpr::Post(cpr::Url{"https://api.digitransit.fi/routing/v1/routers/waltti/index/graphql"},
    cpr::Body{stopPayload.dump()},
    cpr::Header{{"Content-Type", "application/json"}});

    json responseJson = json::parse(r.text)["data"]["stops"];

    json nysseStops = json::array();
    for (json stop : responseJson) {
        if (stop["gtfsId"].dump().find("tampere") != std::string::npos) {
            nysseStops.push_back(stop);
        }
    }
    return nysseStops;
}
    
dpp::message stopSelect(dpp::snowflake channel, std::map<std::string, std::string> stopParams, std::string selectorId) {
    dpp::message message(channel, "");

    json stopList = stopRequest(stopParams["query"]);
    
    if (stopList.size() > 1 && stopList.dump().find("tampere") != std::string::npos) {   
        std::string messageBuilder = "__**Choose stop**__:\n";
        dpp::component menuSelector;    

        menuSelector.set_type(dpp::cot_selectmenu).set_placeholder("Choose stop...").set_id(selectorId);
        for (json stopDict : stopList) {
            if (stopDict["gtfsId"].dump().find("tampere") != std::string::npos) {
                std::string code = stopDict["code"].get<std::string>();
                menuSelector.add_select_option(dpp::select_option(stopDict["name"].get<std::string>(), code, fmt::format("ID: {}", code)));  
            }
        };
        message.set_content(messageBuilder);
        message.add_component(dpp::component().add_component(menuSelector));
        return message;
    } else if (stopList.size() > 0 && stopList[0]["gtfsId"].dump().find("tampere") != std::string::npos) {
        stopParams["query"] = stopList[0]["code"];
        return stopSearch(channel, stopParams);
    } else {
        message.set_content("```No stops found.```");
    }
    return message;
}
