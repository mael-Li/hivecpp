//
// Created by 李世佳 on 24-10-17.
//

#ifndef GAME_H
#define GAME_H
#include "Hive.h"
#include <vector>

class Player;

class Game {
public:
    void start();
private:
    piecetype::Board board;
    std::vector<std::shared_ptr<Player>> players;
    Game();
};

class Player {
public:
    virtual ~Player() {}
    virtual void makeMove(piecetype::Board& board) = 0;
    void setName(std::string name){name = name;}
    std::string getName() const { return name; }
private:
    std::string name;
};

class HumanPlayer : public Player {
public:
    HumanPlayer(std::string name);
    void makeMove(piecetype::Board& board) override;
};

class AIPlayer : public Player {
public:
    AIPlayer(std::string name);
    void makeMove(piecetype::Board& board) override;
};

#endif //GAME_H
