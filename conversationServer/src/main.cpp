//
// Created by andrew on 05.03.26.
//

#include <cstdlib>
#include <iostream>
#include <ostream>
#include "../headers/ConversationServer.h"

int main(int argc,const char** argv) {
    try {
        ConversationServer server({9999,"0.0.0.0"});
        server.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::exit(EXIT_SUCCESS);
}
