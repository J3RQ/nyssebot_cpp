#include "utils.h"

std::vector<std::string> splitString(std::string inputString, char splitChar) {
    std::stringstream wholeString(inputString);
    std::string stringPart;
    std::vector<std::string> paramlist;

    while (std::getline(wholeString, stringPart, splitChar)) {
        paramlist.push_back(stringPart);
    }
    return paramlist;
}