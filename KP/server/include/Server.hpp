#pragma once

#include <atomic>
#include <string>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#include "../../common.hpp"
#include "./GameManager.hpp"

class Server {
public:
    Server();
    ~Server();

    void initialize();
    void run();
    void shutdown();

private:
    bool running_;
    static bool* global_running_flag_;
    int server_fd_;
    GameManager game_manager_;

    Message handleCreateGame(const Message& msg);
    Message handleJoinGame(const Message& msg);
    Message handleLeaveGame(const Message& msg);
    Message handleFindGame(const Message& msg);
    Message handleListGames(const Message& msg);
    Message handleGuessNumber(const Message& msg);
    Message handleGetGameState(const Message& msg);
    Message handleGetGamePlayers(const Message& msg);
    Message handleGetGameStats(const Message& msg);
    Message handleGetGameLog(const Message& msg);
    Message handleQuit(const Message& msg);
    Message handleHelp(const Message& msg);

    void setupSignalHandlers();
    void initializeFIFO();
    void cleanupFIFO();
    void processMessage(const std::string& message);
    void sendResponse(const std::string& client_fifo_path, const Message& response);

    static void signalHandler(int signum);
};
