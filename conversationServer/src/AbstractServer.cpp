#include "../headers/AbstractServer.h"
#include <cstdint>
#include <sys/types.h>

void AbstractServer::initLogging() {
    try {
        spdlog::init_thread_pool(8192, 1);

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_st>("../log.txt", true);

        std::vector<spdlog::sink_ptr> sinks {console_sink, file_sink};

        auto async_logger = std::make_shared<spdlog::async_logger>(
            "main",
            sinks.begin(),
            sinks.end(),
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::block
        );

        async_logger->set_level(spdlog::level::trace);
        async_logger->flush_on(spdlog::level::debug);
        spdlog::set_default_logger(async_logger);
    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Failed to initialize logging: " << ex.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

int AbstractServer::getActiveFd(std::shared_ptr<Client> client) {
    int activeFd = -1;
    switch (this->serverSocket->getServerInfo().type) {
        case ServerType::Conversation:
            activeFd = client->getDescriptors().audioFd;
            break;
        case ServerType::Image:
            activeFd = client->getDescriptors().videoFd;
            break;
        case ServerType::Unknown:
            throw std::runtime_error("AbstractServer::authenticateClient: Unknown server type");
    }
    return activeFd;
}

void AbstractServer::authenticateClient(std::shared_ptr<Client> client) {
    int activeFd = this->getActiveFd(client);
    switch (this->serverSocket->getServerInfo().type) {
        case ServerType::Conversation:
            activeFd = client->getDescriptors().audioFd;
            break;
        case ServerType::Image:
            activeFd = client->getDescriptors().videoFd;
            break;
        case ServerType::Unknown:
            throw std::runtime_error("AbstractServer::authenticateClient: Unknown server type");
    }
    ssize_t totalHeaderBytes = sizeof(ClientAuthHeader);
    ClientAuthHeader authHeader{};
    ssize_t bytesLeft = totalHeaderBytes;
    char* headerBuffer = reinterpret_cast<char*>(&authHeader);
    while(bytesLeft > 0) {
        ssize_t bytesRead = read(activeFd, headerBuffer, bytesLeft);
        if (bytesRead == -1) {
            throw std::runtime_error("Failed to read client auth header");
        }
        bytesLeft -= bytesRead;
        headerBuffer += bytesRead;
    }
    if (authHeader.id == -1) {
        throw std::runtime_error("Invalid client auth header");
    }
    client->setID(authHeader.id);
    spdlog::info("Client authenticated with ID {}", authHeader.id);
    this->sendAuthResponse(client, ServerAuthStatus::Success);
    ClientLogger::getInstance().insert(client);

}

void AbstractServer::sendAuthResponse(std::shared_ptr<Client> client, ServerAuthStatus status) {
    int activeFd = this->getActiveFd(client);
    ServerAuthResponseHeader response{.status = static_cast<uint32_t>(status)};
    ssize_t totalBytes = sizeof(response);
    char* buffer = reinterpret_cast<char*>(&response);
    ssize_t bytesLeft = totalBytes;
    while (bytesLeft > 0) {
        ssize_t bytesWritten = write(activeFd, buffer, bytesLeft);
        if (bytesWritten == -1) {
            throw std::runtime_error("Failed to send auth response");
        }
        bytesLeft -= bytesWritten;
        buffer += bytesWritten;
    }
    spdlog::info("Auth response sent to client");
}
