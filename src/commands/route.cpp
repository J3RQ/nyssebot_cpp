#include "route.h"
#include "utils.h"

using json = nlohmann::json;

json getCoordinates(std::string query) {{
    std::string url = fmt::format("https://nominatim.openstreetmap.org/search.php", query);
    cpr::Response r = cpr::Get(cpr::Url{{url}}, 
    cpr::Parameters{{"q", fmt::format("{{{}}}+pirkanmaa", query)},{"format", "jsonv2"}});
    json error;
    if (r.status_code != 200) return error["error"] = "";
    return json::parse(r.text);
}}

dpp::message route(dpp::snowflake channel, std::map<std::string, std::string> stopParams) {{
    json departure = getCoordinates(stopParams["departure"]);
    json destination = getCoordinates(stopParams["destination"]);
    if (departure.contains("error") || destination.contains("error")) return dpp::message(channel, "```Request to openstreetmap failed!```");
    if (departure == json::array()) return dpp::message(channel, "```Departure location not found!```");
    if (destination == json::array()) return dpp::message(channel, "```Destination coordinates not found!```");

    json routeQuery;

    std::time_t timestamp = std::time(nullptr);

    bool hourEntered = stopParams.count("hour") != 0;
    bool minuteEntered = stopParams.count("minute") != 0;
    bool dateEntered = stopParams.count("date") != 0;
    
    if (hourEntered || minuteEntered || dateEntered) {
        timestamp = paramsToTimestamp(stopParams, hourEntered, minuteEntered, dateEntered);
    }
   
    std::map<std::string, int> dateMap = getTime(timestamp);
    bool arriveBy = (stopParams.count("mode") != 0 && stopParams["mode"] == "arrival") ? true : false; 

    routeQuery["query"] = fmt::format("query {{\n"
        "plan(from: {{lat: {}, lon: {}}}, to: {{lat: {}, lon: {}}}, minTransferTime: 30, numItineraries: 3, walkReluctance: 1, walkSpeed: 1.7, date: \"{}-{}-{}\", time: \"{}:{}:00\", arriveBy: {}) {{\n"
            "itineraries {{\n"
            "legs {{\n"
                "startTime\n"
                "endTime\n"
                "mode\n"
                "duration\n"
                "distance\n"
                "from {{\n"
                    "name\n"
                "}}\n"
                "trip {{\n"
                    "route {{\n"
                        "longName\n"
                        "shortName\n"
                    "}}\n"
                "}}\n"
                "to {{\n"
                    "name\n"
                    "}}\n"
                "}}\n"
            "}}\n"
        "}}\n"
    "}}", departure[0]["lat"], departure[0]["lon"], destination[0]["lat"], destination[0]["lon"],
    dateMap["year"], dateMap["month"], dateMap["day"], dateMap["hour"], dateMap["minute"], arriveBy);

    cpr::Response r = cpr::Post(cpr::Url{"https://api.digitransit.fi/routing/v1/routers/waltti/index/graphql"},
    cpr::Body{routeQuery.dump()},
    cpr::Header{{"Content-Type", "application/json"}});

    json responseJson = json::parse(r.text)["data"]["plan"]["itineraries"];
    if (responseJson == json::array()) return dpp::message(channel, "```No routes found!```");
    std::string message;

    for (json itinerary : responseJson) {
        for (json leg : itinerary["legs"]) {
            std::string mode = leg["mode"].get<std::string>();
            int distance = leg["distance"].get<int>();
            std::string from = leg["from"]["name"].get<std::string>();
            std::string to = leg["to"]["name"].get<std::string>();
            std::string start = cleanTime(leg["startTime"].get<long>()/1000);
            std::string end = cleanTime(leg["endTime"].get<long>()/1000);
            if (leg["mode"].get<std::string>() != std::string("WALK")) {
                std::string line = leg["trip"]["route"]["shortName"].get<std::string>();
                message.append(fmt::format("```{} {} ({} m)\n{} --> {}\n{} --> {}\n```", mode, line, distance, from, to, start, end));
            } else {
                message.append(fmt::format("```{} ({} m)\n{} --> {}\n{} --> {}\n```", mode, distance, from, to, start, end));
            }
        }
        message.append("\n");
    }
    

    return dpp::message(channel, message);
}}
