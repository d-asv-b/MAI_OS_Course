#include "../include/Game.hpp"

#include <iostream>
#include <chrono>
#include <sstream>

Game::Game(const std::string& name, int max_players)
    : name_(name), max_players_(max_players), state_(State::WAITING_FOR_PLAYERS), winner_pid_(-1) {
}

#pragma region Players
bool Game::addPlayer(int player_pid) {
    if (players_.size() < max_players_) {
        players_.insert(player_pid);
        std::cout << "Player " << player_pid << " joined game " << name_ << std::endl;

        if (this->isReadyToStart()) {
            this->startGame();
        }

        return true;
    }

    return false;
}

bool Game::removePlayer(int player_pid) {
    auto it = players_.find(player_pid);

    if (it != players_.end()) {
        players_.erase(it);
        std::cout << "Player " << player_pid << " left game " << name_ << std::endl;

        if (players_.empty()) {
            state_ = State::GAME_OVER;
        }

        return true;
    }

    return false;
}

bool Game::isPlayerInGame(int player_pid) const {
    return players_.count(player_pid) > 0;
}
#pragma endregion

#pragma region GameState
bool Game::isFull() const {
    return players_.size() == max_players_;
}

bool Game::isReadyToStart() const {
    return players_.size() == max_players_;
}

void Game::startGame() {
    if (isReadyToStart()) {
        state_ = State::IN_PROGRESS;
        secret_number_ = generateSecretNumber();
        start_time_ = std::chrono::system_clock::now();
        std::cout << "Game " << name_ << " started. Secret number: " << secret_number_ << std::endl;
    }
}
#pragma endregion

#pragma region GameLogic
std::pair<int, int> Game::guess(int player_pid, const std::string& guess_str) {
    if (state_ != State::IN_PROGRESS || guess_str.length() != 4) {
        return {-1, -1};
    }
    
    std::set<char> digits;
    for (char c : guess_str) {
        if (!isdigit(c) || digits.count(c)) {
            return {-1, -1};
        }

        digits.insert(c);
    }

    player_guesses_[player_pid].push_back(guess_str);
    std::pair<int, int> result = calculateBullsCows(guess_str);
    player_results_[player_pid].push_back(result);

    if (result.first == 4) {
        state_ = State::GAME_OVER;
        winner_pid_ = player_pid;
        std::cout << "Player " << player_pid << " won game " << name_ << "! Secret number was " << secret_number_ << std::endl;
    }

    return result;
}

std::string Game::generateSecretNumber() {
    std::string digits = "0123456789";
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(digits.begin(), digits.end(), g);
    
    // Ensure the first digit is not '0' if it's a 4-digit number and we want non-zero first digit
    if (digits[0] == '0') {
        std::swap(digits[0], digits[1]); // Swap with second digit
    }
    return digits.substr(0, 4);
}

std::pair<int, int> Game::calculateBullsCows(const std::string& guess) const {
    int bulls = 0;
    int cows = 0;

    for (size_t i = 0; i < secret_number_.length(); ++i) {
        if (secret_number_[i] == guess[i]) {
            bulls++;
        } else {
            for (size_t j = 0; j < guess.length(); ++j) {
                if (secret_number_[i] == guess[j]) {
                    cows++;
                    break;
                }
            }
        }
    }

    return {bulls, cows};
}
#pragma endregion

#pragma region Getters
std::vector<int> Game::getPlayers() const {
    return std::vector<int>(players_.begin(), players_.end());
}

std::string Game::getName() const {
    return name_;
}

Game::State Game::getState() const {
    return state_;
}

int Game::getMaxPlayers() const {
    return max_players_;
}


std::string Game::getGameState() const {
    if (state_ == State::WAITING_FOR_PLAYERS) {
        std::stringstream ss;
        ss << "Waiting for players (" << players_.size() << "/" << max_players_ << ")";
        return ss.str();
    } else if (state_ == State::IN_PROGRESS) {
        return "In progress";
    } else {
        return "Game over";
    }
}

std::string Game::getGameStats() const {
    std::stringstream ss;

    if (state_ == State::WAITING_FOR_PLAYERS) {
        ss << "Game not started yet.";
        return ss.str();
    }

    ss << "Game Statistics:\n";
    ss << "Secret number: " << (state_ == State::GAME_OVER ? secret_number_ : "????") << "\n";
    ss << "Winner: " << (winner_pid_ != -1 ? std::to_string(winner_pid_) : "No winner yet") << "\n\n";

    ss << "Player stats:\n";
    for (int player_pid : players_) {
        auto guesses_it = player_guesses_.find(player_pid);
        auto results_it = player_results_.find(player_pid);

        ss << "Player " << player_pid << ":\n";
        ss << "  Total guesses: " << (guesses_it != player_guesses_.end() ? guesses_it->second.size() : 0) << "\n";

        if (results_it != player_results_.end() && !results_it->second.empty()) {
            ss << "  Last result: " << results_it->second.back().first << " bulls, " << results_it->second.back().second << " cows\n";
        }

        if (winner_pid_ == player_pid) {
            ss << "  WINNER!\n";
        }
        ss << "\n";
    }

    return ss.str();
}

std::string Game::getGameLog() const {
    std::stringstream ss;

    if (state_ == State::WAITING_FOR_PLAYERS) {
        ss << "Game not started yet.";
        return ss.str();
    }

    ss << "Game Log:\n";

    for (int player_pid : players_) {
        auto guesses_it = player_guesses_.find(player_pid);
        auto results_it = player_results_.find(player_pid);

        if (guesses_it != player_guesses_.end() && results_it != player_results_.end()) {
            ss << "Player " << player_pid << ":\n";
            const auto& guesses = guesses_it->second;
            const auto& results = results_it->second;

            for (size_t i = 0; i < guesses.size(); ++i) {
                ss << "  Guess " << (i + 1) << ": " << guesses[i]
                   << " -> " << results[i].first << " bulls, " << results[i].second << " cows\n";
            }
            ss << "\n";
        }
    }

    return ss.str();
}

#pragma endregion

