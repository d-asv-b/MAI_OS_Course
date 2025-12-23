#include "../include/GameManager.hpp"

#include <algorithm>
#include <random>
#include <chrono>

std::shared_ptr<Game> GameManager::createGame(const std::string& name, int max_players) {
    std::lock_guard<std::mutex> lock(this->games_mutex_);

    if (this->games_.count(name) == 0) {
        auto new_game = std::make_shared<Game>(name, max_players);
        this->games_[name] = new_game;

        return new_game;
    }

    return nullptr;
}

std::shared_ptr<Game> GameManager::getGame(const std::string& name) {
    std::lock_guard<std::mutex> lock(this->games_mutex_);

    auto it = this->games_.find(name);
    if (it != this->games_.end()) {
        return it->second;
    }

    return nullptr;
}

std::shared_ptr<Game> GameManager::getPlayerCurrentGame(int player_pid) {
    std::lock_guard<std::mutex> lock(this->games_mutex_);

    for (auto const& [name, game] : this->games_) {
        if (game->isPlayerInGame(player_pid)) {
            return game;
        }
    }

    return nullptr;
}

std::shared_ptr<Game> GameManager::findAvailableGame() {
    std::lock_guard<std::mutex> lock(this->games_mutex_);

    for (auto const& [name, game] : this->games_) {
        if (game->getState() == Game::State::WAITING_FOR_PLAYERS && !game->isFull()) {
            return game;
        }
    }

    return nullptr;
}

void GameManager::removeGame(const std::string& name) {
    std::lock_guard<std::mutex> lock(this->games_mutex_);

    this->games_.erase(name);
}

bool GameManager::gameExists(const std::string& name) const {
    std::lock_guard<std::mutex> lock(this->games_mutex_);
    
    return this->games_.count(name) > 0;
}

std::vector<std::string> GameManager::listGames() const {
    std::lock_guard<std::mutex> lock(this->games_mutex_);

    std::vector<std::string> game_names;
    for (auto const& [name, game] : this->games_) {
        game_names.push_back(name + " (" + std::to_string(game->getPlayers().size()) + "/" + std::to_string(game->getMaxPlayers()) + ") - " + game->getGameState());
    }

    return game_names;
}
