//
// Created by andrew on 5/24/26.
//

#pragma once
#include <string>
#include <locale>
class LanguageValidator {
public:
    LanguageValidator() = default;
    static bool validateEnglish(const std::string& text);
    static bool isJunkOrEmpty(const std::string& text);
};