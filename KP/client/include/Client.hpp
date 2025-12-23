#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <memory>

#include "../../common.hpp"

class Client {
public:
    Client();
    ~Client();

    void run();

    void setCurrentGame(const std::string& game_name);
    std::string getCurrentGame() const;

private:
    void setup_signal_handlers();
    void create_client_fifo();
    void cleanup_client_fifo();
    void start_response_reader();
    void stop_response_reader();

    MessageType parse_command(const std::string& command_line, std::string& payload);
    void send_message(MessageType type, const std::string& payload = "");

    static void signal_handler(int signum);
    void read_responses();

    static bool running_;
    int client_pid_;
    std::string client_fifo_path_;
    std::string current_game_name_;
    std::unique_ptr<std::thread> reader_thread_;
};
