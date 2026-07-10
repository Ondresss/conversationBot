//
// Created by andrew on 4/18/26.
//
#include "../headers/ClientLogger.h"
#include <memory>

ClientLogger& ClientLogger::getInstance() {
    static ClientLogger logger("../database.db","../schema.sql");
    return logger;
}

void ClientLogger::insert(const Client& c) {
    const char* sql = "INSERT OR IGNORE INTO client (id, ip,port) VALUES (?, ?,?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(this->db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        spdlog::error("Failed to prepare insert statement: {}", sqlite3_errmsg(this->db));
        return;
    }

    sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(c.getId()));
    sqlite3_bind_text(stmt, 2, c.getIP().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, c.getPort());
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        spdlog::error("Execution failed: {}", sqlite3_errmsg(this->db));
    }

    sqlite3_finalize(stmt);
}

void ClientLogger::insertSpeech(const Client& c, const std::string& question, const std::string& answer) {
    const char* sql = "INSERT INTO history (client_id, question, answer) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(this->db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        spdlog::error("Prepare history insert failed: {}", sqlite3_errmsg(this->db));
        throw std::runtime_error("void ClientLogger::insertSpeech(const Client& c, const std::string& question, const std::string& answer)");
    }

    sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(c.getId()));
    sqlite3_bind_text(stmt, 2, question.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, answer.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        spdlog::error("Step history insert failed: {}", sqlite3_errmsg(this->db));
        throw std::runtime_error("void ClientLogger::insertSpeech(const Client& c, const std::string& question, const std::string& answer)");

    }

    sqlite3_finalize(stmt);
}

std::vector<std::shared_ptr<Client>> ClientLogger::selectAll() {
    std::vector<std::shared_ptr<Client>> clients;

    const char* clientSql = "SELECT id, ip, port FROM client;";
    sqlite3_stmt* clientStmt;

    if (sqlite3_prepare_v2(this->db, clientSql, -1, &clientStmt, nullptr) != SQLITE_OK) {
        spdlog::error("Failed to prepare select client: {}", sqlite3_errmsg(this->db));
        return clients;
    }

    while (sqlite3_step(clientStmt) == SQLITE_ROW) {
        std::size_t id = sqlite3_column_int64(clientStmt, 0);
        std::string ip = reinterpret_cast<const char*>(sqlite3_column_text(clientStmt, 1));
        int port = sqlite3_column_int(clientStmt, 2);

        Client c(Client::Descriptor{.audioFd = -1, .videoFd = -1}, ip, port);
        c.setID(id);
        const char* historySql = "SELECT question, answer FROM history WHERE client_id = ? ORDER BY created_at ASC;";
        sqlite3_stmt* historyStmt;

        if (sqlite3_prepare_v2(this->db, historySql, -1, &historyStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int64(historyStmt, 1, id);

            while (sqlite3_step(historyStmt) == SQLITE_ROW) {
                const char* q = reinterpret_cast<const char*>(sqlite3_column_text(historyStmt, 0));
                const char* a = reinterpret_cast<const char*>(sqlite3_column_text(historyStmt, 1));
                c.addHistoryEntry(q ? q : "", a ? a : "");
            }
            sqlite3_finalize(historyStmt);
        }

        clients.push_back(std::make_shared<Client>(Client::Descriptor{.audioFd = -1, .videoFd = -1}, ip, port));
    }

    sqlite3_finalize(clientStmt);
    return clients;
}

void ClientLogger::setupTables() {
    std::ifstream istream(this->schema);
    if(istream.is_open()) {
        std::stringstream ss;
        ss << istream.rdbuf();

        std::string script = ss.str();
        char* errorMessage = nullptr;
        int result = sqlite3_exec(db, script.c_str(), nullptr, nullptr, &errorMessage);

        if (result != SQLITE_OK) {
            spdlog::error("Error while running schema: {}",errorMessage );
            sqlite3_free(errorMessage);
            throw std::runtime_error("void ClientLogger::setupTables(): " + std::string(errorMessage));
        }
        spdlog::info("Loaded schema file: {}",this->schema);
    } else {
        spdlog::error("Could not open schema file: {}",this->schema);
        throw std::runtime_error("void ClientLogger::setupTables(): Could not open starting SQL script\n");
    }
}


ClientLogger::ClientLogger(const std::string& dbName, const std::string& schema) {
    this->schema = schema;
    if(!sqlite3_open(dbName.c_str(), &this->db)) {
        sqlite3_exec(this->db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
        this->setupTables();
    } else {
        throw std::runtime_error("Could not open SQLLTIE DBb\n");
    }
}
