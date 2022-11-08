#include <dpp/dpp.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include "commands/stop.h"
#include "utils/utils.h"

class Config
{
public:
    std::string token;
    std::string mapBoxApikey;
    void getConfig()
    {   
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
};

int main() {
    Config config;
    config.getConfig();
    dpp::cluster bot(config.token);

    bot.on_log(dpp::utility::cout_logger());
    bot.set_presence(dpp::presence(dpp::ps_online, dpp::activity_type::at_game, "Nysse API"));
    bot.on_slashcommand([&bot](const dpp::slashcommand_t & event) {
        if (event.command.get_command_name() == "stop") {
            std::string params[5] = {"query", "line", "hour", "minute", "date"};
            std::map<std::string, std::string> parameters;
            for (std::string param : params) {
                try {
                    if (param == "query" || param == "line" || param == "date") {
                        parameters.insert(std::pair<std::string, std::string>(param, std::get<std::string>(event.get_parameter(param))));
                    }  else {
                        parameters.insert(std::pair<std::string, std::string>(param, std::to_string(std::get<int64_t>(event.get_parameter(param)))));
                    }
                } catch(const std::exception& e) {
                    bot.log(dpp::ll_info, fmt::format("Parameter {} not entered", param));
                }  
            }
            event.reply(stopMain(event.command.channel_id, parameters));
        }
    });

    bot.on_select_click([&bot](const dpp::select_click_t & event) {
        if (event.custom_id == "stopSelector") {
            std::map<std::string, std::string> params;
            params["query"] = event.values[0];
            event.reply(stopMain(event.command.channel_id, params));
        }
    });
 
    bot.on_ready([&bot](const dpp::ready_t & event) {
        if (dpp::run_once<struct register_bot_commands>()) {
            dpp::slashcommand query("stop", "Get departure timetable from a stop", bot.me.id);
            query.set_dm_permission(true);
            query.add_option(dpp::command_option(dpp::co_string, "query", "Stop ID or name.", true));
            query.add_option(dpp::command_option(dpp::co_string, "line", "Limit results to desired lines. Separate with commas.", false));
            query.add_option(dpp::command_option(dpp::co_integer, "hour", "Hour to search at.", false).set_min_value(0).set_max_value(23));
            query.add_option(dpp::command_option(dpp::co_integer, "minute", "Minute to search at.", false).set_min_value(0).set_max_value(59));
            query.add_option(dpp::command_option(dpp::co_string, "date", "Date. Format DD.MM or alternatively DD.MM.YYYY", false));
            
            bot.global_command_create(query);
        }
    });

    bot.start(dpp::st_wait);
}