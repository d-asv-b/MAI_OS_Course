#include "../include/Client.hpp"

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sstream>

bool Client::running_ = true;

Client::Client()
    : client_pid_(getpid()),
      client_fifo_path_(CLIENT_FIFO_PREFIX + std::to_string(client_pid_)),
      current_game_name_("") {
    setup_signal_handlers();
    create_client_fifo();
    start_response_reader();
}

Client::~Client() {
    running_ = false;
    stop_response_reader();
    cleanup_client_fifo();
}

void Client::run() {
    std::cout << "Bulls and Cows Client started (PID: " << client_pid_ << ")\n";
    std::cout << "Type 'HELP' for commands.\n";

    while (running_) {
        std::string prompt = "> ";
        if (!current_game_name_.empty()) {
            prompt = "[" + current_game_name_ + "]> ";
        }
        std::cout << prompt;
        std::string command_line;
        std::getline(std::cin, command_line);

        if (command_line.empty()) {
            continue;
        }

        std::string payload;
        MessageType msg_type = parse_command(command_line, payload);
        send_message(msg_type, payload);

        if (msg_type == MessageType::QUIT) {
            running_ = false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "Client shut down.\n";
}

void Client::setup_signal_handlers() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
}

void Client::create_client_fifo() {
    remove_FIFO(client_fifo_path_);
    create_FIFO(client_fifo_path_);
    std::cout << "Client FIFO created at " << client_fifo_path_ << "\n";
}

void Client::cleanup_client_fifo() {
    remove_FIFO(client_fifo_path_);
}

void Client::start_response_reader() {
    reader_thread_ = std::make_unique<std::thread>(&Client::read_responses, this);
}

void Client::stop_response_reader() {
    if (reader_thread_ && reader_thread_->joinable()) {
        reader_thread_->join();
    }
}

MessageType Client::parse_command(const std::string& command_line, std::string& payload) {
    MessageType msg_type = MessageType::INVALID_COMMAND;

    if (command_line.rfind("CREATE_GAME ", 0) == 0) {
        msg_type = MessageType::CREATE_GAME;
        payload = command_line.substr(std::string("CREATE_GAME ").length());
    }
    else if (command_line.rfind("JOIN_GAME ", 0) == 0) {
        msg_type = MessageType::JOIN_GAME;
        payload = command_line.substr(std::string("JOIN_GAME ").length());
    }
    else if (command_line == "LEAVE_GAME") {
        msg_type = MessageType::LEAVE_GAME;
        payload = "";
    }
    else if (command_line.rfind("LEAVE_GAME ", 0) == 0) {
        msg_type = MessageType::LEAVE_GAME;
        payload = command_line.substr(std::string("LEAVE_GAME ").length());
    }
    else if (command_line.rfind("GUESS_NUMBER ", 0) == 0) {
        msg_type = MessageType::GUESS_NUMBER;
        payload = command_line.substr(std::string("GUESS_NUMBER ").length());
    }
    else if (command_line == "GET_GAME_STATE") {
        msg_type = MessageType::GET_GAME_STATE;
        payload = "";
    }
    else if (command_line.rfind("GET_GAME_STATE ", 0) == 0) {
        msg_type = MessageType::GET_GAME_STATE;
        payload = command_line.substr(std::string("GET_GAME_STATE ").length());
    }
    else if (command_line == "GET_GAME_PLAYERS") {
        msg_type = MessageType::GET_GAME_PLAYERS;
        payload = "";
    }
    else if (command_line.rfind("GET_GAME_PLAYERS ", 0) == 0) {
        msg_type = MessageType::GET_GAME_PLAYERS;
        payload = command_line.substr(std::string("GET_GAME_PLAYERS ").length());
    }
    else if (command_line == "GET_GAME_STATS") {
        msg_type = MessageType::GET_GAME_STATS;
        payload = "";
    }
    else if (command_line.rfind("GET_GAME_STATS ", 0) == 0) {
        msg_type = MessageType::GET_GAME_STATS;
        payload = command_line.substr(std::string("GET_GAME_STATS ").length());
    }
    else if (command_line == "GET_GAME_LOG") {
        msg_type = MessageType::GET_GAME_LOG;
        payload = "";
    }
    else if (command_line.rfind("GET_GAME_LOG ", 0) == 0) {
        msg_type = MessageType::GET_GAME_LOG;
        payload = command_line.substr(std::string("GET_GAME_LOG ").length());
    }
    else if (command_line == "FIND_GAME") {
        msg_type = MessageType::FIND_GAME;
    }
    else if (command_line == "LIST_GAMES") {
        msg_type = MessageType::LIST_GAMES;
    }
    else if (command_line == "QUIT") {
        msg_type = MessageType::QUIT;
    }
    else if (command_line == "HELP") {
        msg_type = MessageType::HELP;
    }
    else {
        payload = command_line;
    }

    return msg_type;
}

void Client::send_message(MessageType type, const std::string& payload) {
    Message client_msg = {type, client_pid_, payload};
    write_to_FIFO(SERVER_FIFO_PATH, client_msg.toString());
}

void Client::signal_handler(int signum) {
    running_ = false;
    std::cout << "\nClient shutting down...\n";
}

void Client::setCurrentGame(const std::string& game_name) {
    current_game_name_ = game_name;
}

std::string Client::getCurrentGame() const {
    return current_game_name_;
}

void Client::read_responses() {
    int client_fifo_fd = open(client_fifo_path_.c_str(), O_RDONLY | O_NONBLOCK);

    if (client_fifo_fd == -1) {
        perror("open client FIFO for read");
        return;
    }

    while (running_) {
        char buffer[4096];
        ssize_t bytes_read = read(client_fifo_fd, buffer, sizeof(buffer) - 1);

        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            std::string received_str(buffer);

            Message response_msg = Message::fromString(received_str);
            std::cout << "\nServer: " << response_msg.payload << "\n";

            if (response_msg.type == MessageType::GAME_CREATED) {
                std::string response = response_msg.payload;
                size_t start = response.find("'");
                size_t end = response.find("'", start + 1);
                if (start != std::string::npos && end != std::string::npos) {
                    current_game_name_ = response.substr(start + 1, end - start - 1);
                }
            } else if (response_msg.type == MessageType::GAME_JOINED) {
                std::string response = response_msg.payload;
                size_t start = response.find("'");
                size_t end = response.find("'", start + 1);
                if (start != std::string::npos && end != std::string::npos) {
                    current_game_name_ = response.substr(start + 1, end - start - 1);
                }
            } else if (response_msg.type == MessageType::OK &&
                      response_msg.payload.rfind("Left game '", 0) == 0) {
                current_game_name_ = "";
            }
        } else if (bytes_read == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("read from client FIFO");
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    close(client_fifo_fd);
}
