#pragma once

#include <string>
#include <vector>

const std::string SERVER_FIFO_PATH = "/tmp/bc_server_fifo";
const std::string CLIENT_FIFO_PREFIX = "/tmp/bc_client_";

enum class MessageType {
    CREATE_GAME,
    JOIN_GAME,
    LEAVE_GAME,
    FIND_GAME,
    LIST_GAMES,
    GUESS_NUMBER,
    GET_GAME_STATE,
    GET_GAME_PLAYERS,
    GET_GAME_STATS,
    GET_GAME_LOG,
    QUIT,
    HELP,

    GAME_CREATED,
    GAME_JOINED,
    GAME_NOT_FOUND,
    GAME_FULL,
    INVALID_COMMAND,
    SERVER_ERROR,
    OK,
    GAME_STATE,
    GAME_PLAYERS,
    GAME_STATS,
    GAME_LOG,
    WAITING_FOR_PLAYERS,
    GUESS_RESULT,
    GAME_OVER
};

struct Message {
    MessageType type;
    int client_pid;
    std::string payload;

    std::string toString() const;
    static Message fromString(const std::string& s);
};

void create_FIFO(const std::string& path);
void remove_FIFO(const std::string& path);
void write_to_FIFO(const std::string& path, const std::string& message);
std::string read_from_FIFO(const std::string& path);


