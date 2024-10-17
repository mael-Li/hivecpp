#include "Hive.h"
#include "Game.h"
int main() {
        using namespace piecetype;
        Game game;
        game.start();
    return 0;
    }
/*
// 添加棋子
while (pos) {
    std::cout<<"Please input Piece:\n";
    std::cin>>x>>y;
    std::cout<<std::endl;
    C.q = x;
    C.r = y;
    board.addPiece(queen,C);
    board.printBoard();
    std::cout<<"if you want continue,please type 1\n";
    std::cin>>pos;
    if(!pos) break;
}
*/