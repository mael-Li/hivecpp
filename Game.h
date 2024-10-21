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
    std::string getOpponentName(PlayerID player) {
        for(const auto &it : players)
            it->getid()
        return "Unkowned"; // 应该不会到达这里
    }
private:
    Board board;
    std::vector<std::shared_ptr<Player>> players;

};
class Player {
public:
    Player(std::string n,const PlayerID& a):name(n),id(a){}
    virtual ~Player() {}
    virtual void makeMove(piecetype::Board& board,int) = 0;
    void setName(std::string n){name = n;}
    std::string getName() const { return name; }
    PlayerID getid()const{return id;}
    void setid(const PlayerID a){id = a;}
private:
    std::string name;
    PlayerID id;
};
class HumanPlayer : public Player {
public:
    HumanPlayer(std::string ,const PlayerID&);
    void makeMove(Board& board,int) override;
};
class AIPlayer : public Player {
public:
    AIPlayer(std::string);
    void makeMove(Board& board,int) override;
};

#endif //GAME_H
