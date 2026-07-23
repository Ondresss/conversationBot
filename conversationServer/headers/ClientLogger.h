//
// Created by andrew on 4/18/26.
//

#pragma once
#include <string>
#include <sqlite3.h>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include "Client.h"
#include <spdlog/spdlog.h>
class ClientLogger {
public:
    static ClientLogger& getInstance();
    void insert(std::shared_ptr<Client> c);
    void insertSpeech(std::shared_ptr<Client> c, const std::string& question, const std::string& answer);
    std::vector<std::shared_ptr<Client>> selectAll();
private:
    ClientLogger(const std::string& dbName,const std::string& schema);
    void setupTables();
    sqlite3* db = nullptr;
    std::string schema;
};
