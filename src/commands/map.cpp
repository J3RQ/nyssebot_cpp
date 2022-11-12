#include "utils.h"
#include "stop.h"

using json = nlohmann::json;

dpp::message getMap(dpp::snowflake channel, std::map<std::string, std::string> parameters) {
    Config config;
    config.getConfig();
    json stops = stopRequest(parameters["query"]);
    if (stops.size() > 1) {
        return stopSelect(channel, parameters, std::string("mapSelect"));
    } else if (stops.size() == 1) {
        std::string lon = std::to_string(stops[0]["lon"].get<float>());
        std::string lat = std::to_string(stops[0]["lat"].get<float>());

        std::string queryUrl = fmt::format("https://api.mapbox.com/styles/v1/mapbox/streets-v11/static/pin-s+3467fe({},{})/{},{},14.11,0/600x400?access_token={}", lon, lat, lon, lat, config.mapBoxApikey);

        cpr::Response r = cpr::Get(cpr::Url{queryUrl});
        if (r.status_code != 200) return dpp::message().set_content("```Failed to fetch map. Mapbox API may be down?```");

        dpp::message mapImage = dpp::message(fmt::format("__**Map for {} ({})**__", stops[0]["name"].get<std::string>(), stops[0]["code"].get<std::string>()));

        mapImage.add_file(fmt::format("{}.png", stops[0]["code"].get<std::string>()), r.text);

        return mapImage;
    } else {
        return dpp::message().set_content("```No stops found.```");
    }
}

