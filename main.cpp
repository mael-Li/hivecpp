#include "Hive.h"

int main() {
        using namespace piecetype;
        int x,y;
        int index =0;
        bool pos = true;
        Board board(5); // 创建一个大小为5的棋盘
        QueenBee Q;
        HexCoord C;
        board.printBoard();
        std::cout<<"Please input Piece:\n";
        std::cin>>x>>y;
        std::cout<<std::endl;
        C.q = x;
        C.r = y;
    // 添加棋子
    while (pos) {
        std::cout<<"if you want continue,please type 1\n";
        std::cin>>pos;
        if(!pos) break;
        board.addPiece(std::make_shared<QueenBee>(),C);
        board.printBoard();
    }
    return 0;
    }
