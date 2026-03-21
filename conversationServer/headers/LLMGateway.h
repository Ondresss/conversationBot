//
// Created by andrew on 3/21/26.
//

#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <memory>
class LLMGateway {
public:
    struct LLMParams {
        std::string binaryPath;
        int port = -1;
        std::string modelPath;
    };
    struct CurlDeleter {
        void operator()(CURL* curl_) const {
            if (curl_) {
                curl_easy_cleanup(curl_);
            }
        }
    };
    struct CurlListDeleter {
        void operator()(curl_slist* list) const {
            if (list) {
                curl_slist_free_all(list);
            }
        }
    };
    explicit LLMGateway(LLMParams params_) : params(std::move(params_)) {
        this->curl = std::unique_ptr<CURL, CurlDeleter>(curl_easy_init());
        if (!this->curl) {
            throw std::runtime_error("LLMGateway(LLMParams params_): Could not init CURL");
        }
        this->init();
    }
    std::string askLLM(const std::string& text);
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
    static LLMParams parseArgs(int argc,const char** argv);
private:
    LLMParams params;
    std::unique_ptr<CURL,CurlDeleter> curl = nullptr;
    std::unique_ptr<curl_slist, CurlListDeleter> headers = nullptr;
    void init();
};