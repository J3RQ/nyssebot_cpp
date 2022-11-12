#include <dpp/dpp.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include "commands/stop.h"
#include "commands/map.h"
#include "utils/utils.h"

class Eventcache {
public:
    std::map<dpp::snowflake, std::map<std::string, std::string>> eventMap; 

    std::map<std::string, std::string> getEvent(dpp::snowflake messageId) {
        return eventMap[messageId];
    }

    void setEvent(dpp::snowflake messageId, std::map<std::string, std::string> event) {
        eventMap[messageId] = event;
    }

    void removeEvent(dpp::snowflake messageId) {
        eventMap.erase(messageId);
    }
};

int main() {
    Config config;
    Eventcache eventCache;
    config.getConfig();
    dpp::cluster bot(config.token);

    bot.on_log(dpp::utility::cout_logger());
   
    bot.on_slashcommand([&bot, &eventCache](const dpp::slashcommand_t & event) {
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
            dpp::message stopMessage = stopMain(event.command.channel_id, parameters);
            event.reply(stopMessage);
            if (stopMessage.components.empty() == false) {
                event.get_original_response([&bot, &event, &eventCache, stopMessage, parameters] (const dpp::confirmation_callback_t& callback) {
                    if (callback.is_error()) return;
                    dpp::message reply = std::get<dpp::message>(callback.value);    
                    eventCache.setEvent(reply.id, parameters);
                });
            }   
        }
        if (event.command.get_command_name() == "map") {
            std::map<std::string, std::string> parameters;
            parameters["query"] = std::get<std::string>(event.get_parameter("stop"));
            dpp::message mapMessage = getMap(event.command.channel_id, parameters);
            event.reply(mapMessage);
        }
    });

    bot.on_select_click([&bot, &eventCache](const dpp::select_click_t & event) {
        if (event.custom_id == "stopSelector") {
            dpp::message originalMsg = bot.message_get_sync(event.command.message_id, event.command.channel_id);
            if (event.command.usr.id == originalMsg.interaction.usr.id) {
                std::map<std::string, std::string> commandParams;
                try {
                    commandParams = eventCache.getEvent(event.command.message_id);
                    commandParams["query"] = event.values[0];
                } catch(const std::exception& e) {
                    commandParams["query"] = event.values[0];
                }
                originalMsg.components.clear();
                originalMsg.set_content(stopMain(event.command.channel_id, commandParams).content);
                bot.message_edit_sync(originalMsg);
                eventCache.removeEvent(event.command.message_id);
                event.cancel_event();
            } else {
                dpp::message wrongUser = dpp::message(event.command.channel_id, "You didn't call this embed!");
                wrongUser.set_flags(dpp::m_ephemeral);
                event.reply(wrongUser);
            }
        }
        if (event.custom_id == "mapSelect") {
            dpp::message originalMsg = bot.message_get_sync(event.command.message_id, event.command.channel_id);
            if (event.command.usr.id == originalMsg.interaction.usr.id) {
                std::map<std::string, std::string> params;
                params["query"] = event.values[0];
                dpp::message mapMessage = getMap(event.command.channel_id, params);
                originalMsg.components.clear();
                originalMsg.set_content(mapMessage.content);
                originalMsg.add_file(mapMessage.filename[0], mapMessage.filecontent[0]);
                bot.message_edit_sync(originalMsg);
                event.cancel_event();
            }
        }
    });
 
    bot.on_ready([&bot](const dpp::ready_t & event) {
        bot.set_presence(dpp::presence(dpp::ps_online, dpp::activity_type::at_game, "Nysse API"));
        if (dpp::run_once<struct register_bot_commands>()) {

            dpp::slashcommand query("stop", "Get departure timetable from a stop", bot.me.id);
            query.set_dm_permission(true);
            query.add_option(dpp::command_option(dpp::co_string, "query", "Stop ID or name.", true));
            query.add_option(dpp::command_option(dpp::co_string, "line", "Limit results to desired lines. Separate with commas.", false));
            query.add_option(dpp::command_option(dpp::co_integer, "hour", "Hour to search at.", false).set_min_value(0).set_max_value(23));
            query.add_option(dpp::command_option(dpp::co_integer, "minute", "Minute to search at.", false).set_min_value(0).set_max_value(59));
            query.add_option(dpp::command_option(dpp::co_string, "date", "Date. Format DD.MM or alternatively DD.MM.YYYY", false).set_max_length(10));

            dpp::slashcommand map("map", "Get map of stop", bot.me.id);
            map.add_option(dpp::command_option(dpp::co_string, "stop", "Stop ID or name.", true));
            map.set_dm_permission(true);
            
            std::vector<dpp::slashcommand> commands = {query, map};
            bot.global_bulk_command_create(commands);
        }
    });

    bot.start(dpp::st_wait);
}