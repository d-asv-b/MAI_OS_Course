#include "../include/Server.hpp"

#include <iostream>
#include <sstream>

bool* Server::global_running_flag_ = nullptr;

Server::Server() : running_(true), server_fd_(-1) {
    global_running_flag_ = &this->running_;
}

Server::~Server() {
    shutdown();
}

#pragma region Utilities
void Server::signalHandler(int signum) {
    if (global_running_flag_) {
        *global_running_flag_ = false;
        std::cout << "\nServer shutting down...\n";
    }
}

void Server::setupSignalHandlers() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
}

void Server::initializeFIFO() {
    remove_FIFO(SERVER_FIFO_PATH);
    create_FIFO(SERVER_FIFO_PATH);

    std::cout << "Server FIFO created at " << SERVER_FIFO_PATH << "\n";
}

void Server::cleanupFIFO() {
    if (this->server_fd_ != -1) {
        close(this->server_fd_);
        this->server_fd_ = -1;
    }

    remove_FIFO(SERVER_FIFO_PATH);
}

void Server::sendResponse(const std::string& client_fifo_path, const Message& response) {
    write_to_FIFO(client_fifo_path, response.toString());
}

void Server::processMessage(const std::string& message) {
    Message msg = Message::fromString(message);
    std::string client_fifo_path = CLIENT_FIFO_PREFIX + std::to_string(msg.client_pid);

    Message response = { MessageType::SERVER_ERROR, 0, "Unknown command" };

    switch (msg.type) {
        case MessageType::CREATE_GAME:
            response = handleCreateGame(msg);
            break;
        case MessageType::JOIN_GAME:
            response = handleJoinGame(msg);
            break;
        case MessageType::LEAVE_GAME:
            response = handleLeaveGame(msg);
            break;
        case MessageType::FIND_GAME:
            response = handleFindGame(msg);
            break;
        case MessageType::LIST_GAMES:
            response = handleListGames(msg);
            break;
        case MessageType::GUESS_NUMBER:
            response = handleGuessNumber(msg);
            break;
        case MessageType::GET_GAME_STATE:
            response = handleGetGameState(msg);
            break;
        case MessageType::GET_GAME_PLAYERS:
            response = handleGetGamePlayers(msg);
            break;
        case MessageType::GET_GAME_STATS:
            response = handleGetGameStats(msg);
            break;
        case MessageType::GET_GAME_LOG:
            response = handleGetGameLog(msg);
            break;
        case MessageType::QUIT:
            response = handleQuit(msg);
            break;
        case MessageType::HELP:
            response = handleHelp(msg);
            break;  
        default:
            response = { MessageType::INVALID_COMMAND, 0, "Unknown command type: " + std::to_string(static_cast<int>(msg.type)) };
            break;
    }

    sendResponse(client_fifo_path, response);
}
#pragma endregion

#pragma region MainLogic
void Server::initialize() {
    setupSignalHandlers();
    initializeFIFO();

    this->server_fd_ = open(SERVER_FIFO_PATH.c_str(), O_RDONLY | O_NONBLOCK);
    if (this->server_fd_ == -1) {
        if (errno != ENXIO) {
            perror("open server FIFO for read");
        }
    }

    std::cout << "Bulls and Cows Server starting...\n";
}

void Server::shutdown() {
    this->running_ = false;
    cleanupFIFO();

    std::cout << "Bulls and Cows Server shut down.\n";
}

void Server::run() {
    while (this->running_) {
        char buffer[4096];
        ssize_t bytes_read = read(this->server_fd_, buffer, sizeof(buffer) - 1);

        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            std::string received_str(buffer);
            std::cout << "Received: " << received_str << "\n";

            processMessage(received_str);
        } else if (bytes_read == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("read from server FIFO");
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
#pragma endregion

#pragma region MessageHandlers  
Message Server::handleCreateGame(const Message& msg) {
    std::stringstream ss(msg.payload);
    std::string game_name;
    int num_players;
    ss >> game_name >> num_players;

    if (game_name.empty() || num_players <= 0) {
        return { MessageType::INVALID_COMMAND, 0, "Usage: CREATE_GAME <game_name> <num_players>" };
    }

    auto current_game = this->game_manager_.getPlayerCurrentGame(msg.client_pid);
    if (current_game) {
        return { MessageType::SERVER_ERROR, 0, "You are already in game '" + current_game->getName() + "'. Leave it first." };
    }

    auto new_game = this->game_manager_.createGame(game_name, num_players);
    if (new_game) {
        new_game->addPlayer(msg.client_pid);
        return { MessageType::GAME_CREATED, 0, "Game '" + game_name + "' created with " + std::to_string(num_players) + " players. You have joined the game." };
    } else {
        return { MessageType::SERVER_ERROR, 0, "Game '" + game_name + "' already exists." };
    }
}

Message Server::handleJoinGame(const Message& msg) {
    std::string game_name = msg.payload;

    if (game_name.empty()) {
        return { MessageType::INVALID_COMMAND, 0, "Usage: JOIN_GAME <game_name>" };
    }

    auto current_game = this->game_manager_.getPlayerCurrentGame(msg.client_pid);
    if (current_game) {
        return { MessageType::SERVER_ERROR, 0, "You are already in game '" + current_game->getName() + "'. Leave it first." };
    }

    std::shared_ptr<Game> game = this->game_manager_.getGame(game_name);

    if (game) {
        if (!game->isFull()) {
            game->addPlayer(msg.client_pid);
            return { MessageType::GAME_JOINED, 0, "Joined game '" + game_name + "'. " + game->getGameState() };
        } else {
            return { MessageType::GAME_FULL, 0, "Game '" + game_name + "' is full." };
        }
    } else {
        return { MessageType::GAME_NOT_FOUND, 0, "Game '" + game_name + "' not found." };
    }
}

Message Server::handleLeaveGame(const Message& msg) {
    std::string game_name = msg.payload;

    std::shared_ptr<Game> game;
    if (game_name.empty()) {
        game = this->game_manager_.getPlayerCurrentGame(msg.client_pid);
        if (!game) {
            return { MessageType::SERVER_ERROR, 0, "You are not in any game." };
        }
    } else {
        game = this->game_manager_.getGame(game_name);
        if (!game) {
            return { MessageType::GAME_NOT_FOUND, 0, "Game '" + game_name + "' not found." };
        }
    }

    if (game->isPlayerInGame(msg.client_pid)) {
        game->removePlayer(msg.client_pid);
        return { MessageType::OK, 0, "Left game '" + game->getName() + "'." };
    } else {
        return { MessageType::SERVER_ERROR, 0, "You are not in game '" + game->getName() + "'." };
    }
}

Message Server::handleFindGame(const Message& msg) {
    std::shared_ptr<Game> available_game = this->game_manager_.findAvailableGame();

    if (available_game) {
        if (!available_game->isFull()) {
            available_game->addPlayer(msg.client_pid);
            return { MessageType::GAME_JOINED, 0, "Joined game '" + available_game->getName() + "'. " + available_game->getGameState() };
        } else {
            return { MessageType::GAME_FULL, 0, "Game '" + available_game->getName() + "' is now full." };
        }
    } else {
        return { MessageType::OK, 0, "No available games to join. Try creating a new game with CREATE_GAME." };
    }
}

Message Server::handleListGames(const Message& msg) {
    std::vector<std::string> games_list = this->game_manager_.listGames();
    std::string games_str = "Available games:\n";

    if (games_list.empty()) {
        games_str += "  No games currently available.";
    } else {
        for (const std::string& game_info : games_list) {
            games_str += "  - " + game_info + "\n";
        }
    }

    return { MessageType::OK, 0, games_str };
}

Message Server::handleGuessNumber(const Message& msg) {
    std::stringstream ss(msg.payload);
    std::string game_name;
    std::string guess_str;
    ss >> game_name >> guess_str;

    std::shared_ptr<Game> game;
    if (game_name.empty() || (ss.eof() && game_name.length() == 4 && std::all_of(game_name.begin(), game_name.end(), ::isdigit))) {
        if (game_name.length() == 4 && std::all_of(game_name.begin(), game_name.end(), ::isdigit)) {
            guess_str = game_name;
            game_name = "";
        }

        game = this->game_manager_.getPlayerCurrentGame(msg.client_pid);
        if (!game) {
            return { MessageType::SERVER_ERROR, 0, "You are not in any game." };
        }
    } else {
        game = this->game_manager_.getGame(game_name);
        if (!game) {
            return { MessageType::GAME_NOT_FOUND, 0, "Game '" + game_name + "' not found." };
        }
    }

    if (guess_str.empty()) {
        return { MessageType::INVALID_COMMAND, 0, "Usage: GUESS_NUMBER [game_name] <number>" };
    }

    if (game->isPlayerInGame(msg.client_pid)) {
        auto result = game->guess(msg.client_pid, guess_str);
        if (result.first == -1 && result.second == -1) {
            return { MessageType::INVALID_COMMAND, 0, "Invalid guess format. Must be 4 unique digits." };
        } else {
            return { MessageType::GUESS_RESULT, 0, "Guess '" + guess_str + "': " + std::to_string(result.first) + " bulls, " + std::to_string(result.second) + " cows" };
        }
    } else {
        return { MessageType::SERVER_ERROR, 0, "You are not in game '" + game->getName() + "'." };
    }
}

Message Server::handleGetGameState(const Message& msg) {
    std::string game_name = msg.payload;

    std::shared_ptr<Game> game;
    if (game_name.empty()) {
        game = this->game_manager_.getPlayerCurrentGame(msg.client_pid);
        if (!game) {
            return { MessageType::SERVER_ERROR, 0, "You are not in any game." };
        }
    } else {
        game = this->game_manager_.getGame(game_name);
        if (!game) {
            return { MessageType::GAME_NOT_FOUND, 0, "Game '" + game_name + "' not found." };
        }
    }

    return { MessageType::GAME_STATE, 0, game->getGameState() };
}

Message Server::handleGetGamePlayers(const Message& msg) {
    std::string game_name = msg.payload;

    std::shared_ptr<Game> game;
    if (game_name.empty()) {
        game = this->game_manager_.getPlayerCurrentGame(msg.client_pid);
        if (!game) {
            return { MessageType::SERVER_ERROR, 0, "You are not in any game." };
        }
    } else {
        game = this->game_manager_.getGame(game_name);
        if (!game) {
            return { MessageType::GAME_NOT_FOUND, 0, "Game '" + game_name + "' not found." };
        }
    }

    std::vector<int> players = game->getPlayers();
    std::string players_str = "Players in '" + game->getName() + "': ";
    for (size_t i = 0; i < players.size(); ++i) {
        players_str += std::to_string(players[i]);
        if (i < players.size() - 1) players_str += ", ";
    }
    if (players.empty()) players_str += "none";

    return { MessageType::GAME_PLAYERS, 0, players_str };
}

Message Server::handleGetGameStats(const Message& msg) {
    std::string game_name = msg.payload;

    std::shared_ptr<Game> game;
    if (game_name.empty()) {
        game = this->game_manager_.getPlayerCurrentGame(msg.client_pid);
        if (!game) {
            return { MessageType::SERVER_ERROR, 0, "You are not in any game." };
        }
    } else {
        game = this->game_manager_.getGame(game_name);
        if (!game) {
            return { MessageType::GAME_NOT_FOUND, 0, "Game '" + game_name + "' not found." };
        }
    }

    return { MessageType::GAME_STATS, 0, game->getGameStats() };
}

Message Server::handleGetGameLog(const Message& msg) {
    std::string game_name = msg.payload;

    std::shared_ptr<Game> game;
    if (game_name.empty()) {
        game = this->game_manager_.getPlayerCurrentGame(msg.client_pid);
        if (!game) {
            return { MessageType::SERVER_ERROR, 0, "You are not in any game." };
        }
    } else {
        game = this->game_manager_.getGame(game_name);
        if (!game) {
            return { MessageType::GAME_NOT_FOUND, 0, "Game '" + game_name + "' not found." };
        }
    }

    return { MessageType::GAME_LOG, 0, game->getGameLog() };
}

Message Server::handleQuit(const Message& msg) {
    return { MessageType::OK, 0, "Goodbye!" };
}

Message Server::handleHelp(const Message& msg) {
    std::string help_text = "Available commands:\n";
    help_text += "CREATE_GAME <game_name> <num_players> - Create a new game and join it automatically\n";
    help_text += "JOIN_GAME <game_name> - Join an existing game\n";
    help_text += "LEAVE_GAME [game_name] - Leave current game or specified game\n";
    help_text += "LIST_GAMES - List all available games\n";
    help_text += "FIND_GAME - Join the first available game\n";
    help_text += "GUESS_NUMBER <number> - Make a guess in current game (4 unique digits)\n";
    help_text += "GET_GAME_STATE [game_name] - Get state of current or specified game\n";
    help_text += "GET_GAME_PLAYERS [game_name] - Get players in current or specified game\n";
    help_text += "GET_GAME_STATS [game_name] - Get stats of current or specified game\n";
    help_text += "GET_GAME_LOG [game_name] - Get log of current or specified game\n";
    help_text += "QUIT - Quit the client\n";
    help_text += "HELP - Show this help\n";
    help_text += "\nNote: When in a game, you can omit game names for game-specific commands.";

    return { MessageType::OK, 0, help_text };
}
#pragma endregion
