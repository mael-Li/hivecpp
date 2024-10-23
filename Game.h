//
// Created by 李世佳 on 24-10-17.
//

#ifndef GAME_H
#define GAME_H
#include "Hive.h"
#include <utility>
#include <vector>
using namespace piecetype;
class Player {
public:
    Player(std::string n,const PlayerID& a):name(std::move(n)),id(a){}
    virtual ~Player(){}
    virtual void makeMove(piecetype::Board& board,int) = 0;
    void setName(const std::string &n){name = n;}
    std::string getName() const { return name; }
    PlayerID getid()const{return id;}
private:
    std::string name;
    PlayerID id;
};

class Game {
public:
    void start();
    Game();
private:
    Board board;
    std::vector<std::shared_ptr<Player>> players;
    std::string getOpponentName(PlayerID playerId) const {
        for (const auto& it : players) {
            if (it->getid() != playerId) {
                return it->getName();
            }
        }
        return "Unknown"; // 如果找不到对手，返回 "Unknown" （理论上不应该到达这里）
    }
};

class HumanPlayer : public Player {
public:
    HumanPlayer(std::string ,const PlayerID&);
    void makeMove(Board& board,int) override;
};
class AIPlayer : public Player {
public:
    AIPlayer(std::string,const PlayerID&);
    void makeMove(Board& board,int) override;
};

#endif //GAME_H
