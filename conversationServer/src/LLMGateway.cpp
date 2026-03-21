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
    json["prompt"] = "<|im_start|>system\nYou are friendly assistant.<|im_end|>\n"
                 "<|im_start|>user\n" + text + "<|im_end|>\n"
                 "<|im_start|>assistant\n";
    json["n_predict"] = 256;
    std::string jsonData = json.dump();

    curl_easy_setopt(this->curl.get(), CURLOPT_WRITEFUNCTION, LLMGateway::writeCallback);
    curl_easy_setopt(this->curl.get(), CURLOPT_WRITEDATA, &readBuffer);

    curl_easy_setopt(this->curl.get(), CURLOPT_POSTFIELDS, jsonData.c_str());

    CURLcode res = curl_easy_perform(this->curl.get());

    if (res != CURLE_OK) {
        throw std::runtime_error("LLMGateway::sendText: Error while sending request");
    }

    try {
        auto responseJson = nlohmann::json::parse(readBuffer);
        return responseJson["content"].get<std::string>();
    } catch (...) {
        std::cerr << "Error while sending request" << std::endl;
        std::exit(EXIT_FAILURE);
    }
}


LLMGateway::LLMParams LLMGateway::parseArgs(int argc, const char** argv) {
    LLMParams params;
    for (int i = 0; i < argc; ++i) {
        if (!std::strcmp(argv[i],"-llm")) {
            if (argc < 5) throw std::runtime_error("LLMGateway::parseArgs(int argc, const char** argv): Invalid number of arguments with -llm");
            params.port = std::stoi(argv[i+1]);
            params.binaryPath = argv[i+2];
            params.modelPath = argv[i+3];
        }
    }

    return params;
}

void LLMGateway::init() {
    std::stringstream ss;
    ss << "http://" << "localhost:" << std::to_string(this->params.port) << "/completion";
    curl_easy_setopt(this->curl.get(), CURLOPT_URL, ss.str().c_str());
    this->headers = std::unique_ptr<curl_slist, CurlListDeleter>(curl_slist_append(nullptr, "Content-Type: application/json"));
    curl_easy_setopt(this->curl.get(), CURLOPT_HTTPHEADER, this->headers.get());
}
