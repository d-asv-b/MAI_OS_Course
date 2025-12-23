#pragma once

#include <string>
#include <vector>
#include <set>
#include <random>
#include <algorithm>
#include <map>
#include <chrono>

class Game {
public:
    enum class State {
        WAITING_FOR_PLAYERS,
        IN_PROGRESS,
        GAME_OVER
    };

    Game(const std::string& name, int max_players);

    bool addPlayer(int player_pid);
    bool removePlayer(int player_pid);
    bool isPlayerInGame(int player_pid) const;

    bool isFull() const;
    bool isReadyToStart() const;
    void startGame();
    
    std::pair<int, int> guess(int player_pid, const std::string& guess_str);
    std::string getGameState() const;
    std::vector<int> getPlayers() const;
    std::string getName() const;
    State getState() const;
    int getMaxPlayers() const;

    std::string getGameStats() const;
    std::string getGameLog() const;
    
private:
    std::string name_;
    int max_players_;
    std::set<int> players_;
    State state_;
    std::string secret_number_;
    std::map<int, std::vector<std::string>> player_guesses_;
    std::map<int, std::vector<std::pair<int, int>>> player_results_;
    int winner_pid_;
    std::chrono::system_clock::time_point start_time_;

    std::string generateSecretNumber();
    std::pair<int, int> calculateBullsCows(const std::string& guess) const;
};
