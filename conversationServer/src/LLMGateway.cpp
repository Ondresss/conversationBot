//
// Created by andrew on 3/21/26.
//

#include "../headers/LLMGateway.h"

#include <iostream>

std::size_t LLMGateway::writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}

std::string LLMGateway::askLLM(const std::string& text) {

    std::string readBuffer;
    nlohmann::json json;

    nlohmann::json messages = nlohmann::json::array();

    if (this->params.language == "en") {
        if (!LanguageValidator::validateEnglish(text)) {
            return "IGNORE";
        }
        messages.push_back({
    {"role", "system"},
    {"content", "You are a helpful voice assistant. Speak English only.\n\n"
                "RULES:\n"
                "1. It is OK if the user's English grammar is imperfect or broken. Respond normally.\n"
                "2. If the input is completely in another language (like Czech, Chinese) or is pure trash/symbols, you MUST reply with ONLY the single word: IGNORE."
                "3. If the user asks about your age, name, or identity, just say you are an AI assistant and you don't have an age.\n"}
        });

    } else if (this->params.language == "cs") {
        messages.push_back({
          {"role", "system"},
          {"content", "Jsi mluvící hračka. Odpovídej česky, kamarádsky a velmi stručně (1-2 věty).\n"
                      "PRAVIDLO: Pokud text nedává smysl, je to cizí jazyk, nebo jen jedno náhodné slovo (např. '*Svělí*'), "
                      "odpověz POUZE slovem: IGNORE\n"
                      "Příklad: 'Ahoj' -> 'Ahoj kamaráde!'; '*Svělí*' -> IGNORE; 'Hello' -> IGNORE"}
     });
    }

    messages.push_back({{"role", "user"}, {"content", text}});

    json["messages"] = messages;
    json["max_tokens"] = 256;
    json["stream"] = false;

    std::string jsonData = json.dump();


    curl_easy_setopt(this->curl.get(), CURLOPT_WRITEFUNCTION, LLMGateway::writeCallback);
    curl_easy_setopt(this->curl.get(), CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(this->curl.get(), CURLOPT_POSTFIELDS, jsonData.c_str());

    CURLcode res = curl_easy_perform(this->curl.get());

    if (res != CURLE_OK) {
        throw std::runtime_error("LLMGateway::sendText: Error while sending request via Curl");
    }

    try {
        auto responseJson = nlohmann::json::parse(readBuffer);
        if (responseJson.contains("error")) {
            std::cerr << "Server returned API error: " << responseJson["error"]["message"].get<std::string>() << std::endl;
            std::exit(EXIT_FAILURE);
        }

        return responseJson["choices"][0]["message"]["content"].get<std::string>();

    } catch (const std::exception& e) {
        std::cerr << "JSON parsing/HTTP error: " << e.what() << std::endl;
        std::cerr << "Raw server response was: " << readBuffer << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

LLMGateway::LLMParams LLMGateway::parseArgs(int argc, const char** argv) {
    LLMParams params;
    for (int i = 0; i < argc; ++i) {
        if (!std::strcmp(argv[i],"-llm")) {
            if (argc < 6) throw std::runtime_error("LLMGateway::parseArgs(int argc, const char** argv): Invalid number of arguments with -llm");
            params.port = std::stoi(argv[i+1]);
            params.binaryPath = argv[i+2];
            params.modelPath = argv[i+3];
            params.language = argv[i+4];
        }
    }

    return params;
}

void LLMGateway::init() {
    std::stringstream ss;
    ss << "http://127.0.0.1:" << std::to_string(this->params.port) << "/v1/chat/completions";
    curl_easy_setopt(this->curl.get(), CURLOPT_URL, ss.str().c_str());

    this->headers = std::unique_ptr<curl_slist, CurlListDeleter>(curl_slist_append(nullptr, "Content-Type: application/json"));
    curl_easy_setopt(this->curl.get(), CURLOPT_HTTPHEADER, this->headers.get());
}