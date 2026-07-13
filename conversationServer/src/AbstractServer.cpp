#include "../headers/AbstractServer.h"

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


void AbstractServer::authenticateClient(std::shared_ptr<Client> client) {
    ssize_t totalHeaderBytes = sizeof(ClientAuthHeader);
    ClientAuthHeader authHeader{};
    ssize_t bytesLeft = totalHeaderBytes;
    char* headerBuffer = reinterpret_cast<char*>(&authHeader);
    while(bytesLeft > 0) {
        ssize_t bytesRead = read(client->getDescriptors().audioFd, headerBuffer, bytesLeft);
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
}
