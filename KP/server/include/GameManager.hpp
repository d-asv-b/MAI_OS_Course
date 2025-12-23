#pragma once

#include <string>
#include <map>
#include <memory>
#include <mutex>
#include "Game.hpp"

class GameManager {
public:
    GameManager() = default;

    std::shared_ptr<Game> createGame(const std::string& name, int max_players);
    std::shared_ptr<Game> getGame(const std::string& name);
    std::shared_ptr<Game> getPlayerCurrentGame(int player_pid);
    std::shared_ptr<Game> findAvailableGame();

    void removeGame(const std::string& name);
    bool gameExists(const std::string& name) const;

    std::vector<std::string> listGames() const;

private:
    std::map<std::string, std::shared_ptr<Game>> games_;
    mutable std::mutex games_mutex_;
};


