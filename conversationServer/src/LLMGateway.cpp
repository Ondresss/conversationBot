//
// Created by andrew on 3/21/26.
//

#include "../headers/LLMGateway.h"

#include <iostream>
#include <spdlog/spdlog.h>

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
        messages.push_back({
            {"role", "system"},
            {"content", "You are a helpful voice assistant. Speak English only.\n\n"
                        "RULES:\n"
                        "1. You will receive inputs from multiple sources (camera view and conversation text).\n"
                        "2. Combine the inputs to generate a coherent response.\n"
                        "3. If the object is a person, always respond as if you are seeing them in person."}
        });
    } else if (this->params.language == "cs") {
        messages.push_back({
            {"role", "system"},
            {"content", "Jsi mluvící hračka. Odpovídej česky, kamarádsky a velmi stručně (1-2 věty).\n"}
        });
    }

    messages.push_back({{"role", "user"}, {"content", text}});

    if (!this->params.model.empty()) {
        json["model"] = this->params.model;
    } else {
        throw std::runtime_error("LLM doesn't have a model specified");
    }

    json["messages"] = messages;
    json["max_tokens"] = 256;
    json["stream"] = false; 

    std::string jsonData = json.dump();

    curl_easy_setopt(this->curl.get(), CURLOPT_WRITEFUNCTION, LLMGateway::writeCallback);
    curl_easy_setopt(this->curl.get(), CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(this->curl.get(), CURLOPT_POSTFIELDS, jsonData.c_str());
    curl_easy_setopt(this->curl.get(), CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(this->curl.get());

    if (res != CURLE_OK) {
        spdlog::error("CURL error: {}", curl_easy_strerror(res));
        throw std::runtime_error("LLMGateway::askLLM: Error sending request via Curl");
    }

    nlohmann::json responseJson;
    try {
        responseJson = nlohmann::json::parse(readBuffer);
    } catch (const std::exception& e) {
        spdlog::error("Failed to parse JSON response: {}. Buffer content: {}", e.what(), readBuffer);
        throw std::runtime_error("Invalid JSON response from LLM server");
    }

    if (responseJson.contains("error")) {
        std::string errMsg;
        if (responseJson["error"].is_string()) {
            errMsg = responseJson["error"].get<std::string>();
        } else if (responseJson["error"].is_object() && responseJson["error"].contains("message")) {
            errMsg = responseJson["error"]["message"].get<std::string>();
        } else {
            errMsg = responseJson["error"].dump();
        }
        spdlog::error("Server returned API error: {}", errMsg);
        throw std::runtime_error("Server returned API error: " + errMsg);
    }

    if (!responseJson.contains("choices") || !responseJson["choices"].is_array() || responseJson["choices"].empty()) {
        spdlog::error("Unexpected response structure: {}", responseJson.dump());
        throw std::runtime_error("Invalid response layout from LLM server");
    }

    return responseJson["choices"][0]["message"]["content"].get<std::string>();
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
    ss << "http://" << this->params.ip << ":" << std::to_string(this->params.port) << "/v1/chat/completions";
    curl_easy_setopt(this->curl.get(), CURLOPT_URL, ss.str().c_str());

    this->headers = std::unique_ptr<curl_slist, CurlListDeleter>(curl_slist_append(nullptr, "Content-Type: application/json"));
    curl_easy_setopt(this->curl.get(), CURLOPT_HTTPHEADER, this->headers.get());
}
