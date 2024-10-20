//
// Created by 李世佳 on 24-10-17.
//

#ifndef GAME_H
#define GAME_H
#include "Hive.h"
#include <vector>
using namespace piecetype;
class Player;
class Game {
public:
    void start();
    Game();
private:
    Board board;
    std::vector<std::shared_ptr<Player>> players;

};
class Player {
public:
    virtual ~Player() {}
    virtual void makeMove(piecetype::Board& board,int) = 0;
    void setName(std::string n){name = n;}
    std::string getName() const { return name; }
    piecetype::PlayerID getid()const{return id;}
    void setid(const piecetype::PlayerID a){id = a;}
private:
    std::string name;
    piecetype::PlayerID id;
};
class HumanPlayer : public Player {
public:
    HumanPlayer(std::string ,PlayerID);
    void makeMove(Board& board,int) override;
};
class AIPlayer : public Player {
public:
    AIPlayer(std::string name);
    void makeMove(Board& board,int) override;
};

#endif //GAME_H
