//
// Created by andrew on 5/24/26.
//
#include "../headers/LanguageValidator.h"

bool LanguageValidator::validateEnglish(const std::string& text) {
    std::locale loc("en_US.UTF-8");

    return std::all_of(text.begin(), text.end(), [&loc](char c) {
        return std::isalpha(c, loc) || std::isspace(c, loc) || std::ispunct(c, loc);
    });

}

bool LanguageValidator::isJunkOrEmpty(const std::string& text) {
    if (text.empty()) return true;

    for (char c : text) {
        if (std::isalnum(static_cast<unsigned   char>(c))) {
            return false;
        }
    }
    return true;
}
