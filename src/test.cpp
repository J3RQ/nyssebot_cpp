#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

std::vector<std::string> splitString(std::string inputString, char splitChar) {

    std::stringstream wholeString(inputString);
    std::string stringPart;
    std::vector<std::string> paramlist;

    while(std::getline(wholeString, stringPart, splitChar)) {
    paramlist.push_back(stringPart);
    }

    return paramlist;
}

class Config {
public:
    std::string token;
    std::string mapBoxApikey;
    void getConfig() {
        std::ifstream ifs(".env");
        std::string content((std::istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());

        std::vector<std::string> configs = splitString(content, '\n');
        
        std::map<std::string, std::string> configmap;
        std::string possibleParams[2] = {"TOKEN", "MB_APIKEY"};

        for (std::size_t i = 0; i < configs.size(); i++) 
        {
            std::vector<std::string> splitParam = splitString(configs[i], '=');  
            if (std::find(std::begin(possibleParams), std::end(possibleParams), splitParam[0]) != std::end(possibleParams))
            {
                configmap.insert(std::pair<std::string, std::string>(splitParam[0], splitParam[1]));
            }   
        } 
        token = configmap.at("TOKEN");
        mapBoxApikey = configmap.at("MB_APIKEY");
    }
};

int main(int argc, char const *argv[])
{
    Config config;
    config.getConfig();
    std::cout << config.token << "\n";
    std::cout << config.mapBoxApikey << "\n";
    return 0;
}