#include "Hive.h"

int main() {
        using namespace piecetype;
        int x,y;
        int index =0;
        bool pos = true;
        Board board(5); // 创建一个大小为5的棋盘
        QueenBee Q;
        HexCoord C;
        HexCoord newposition;
        //打印一个空棋盘检查
        board.printBoard();
        //初始化蜂后
        auto queen = std::make_shared<QueenBee>();
        std::cout<<"Please input Piece Queen:\n";
        std::cin>>x>>y;
        std::cout<<std::endl;
        C.q = x;
        C.r = y;
        board.addPiece(queen,C);
        board.printBoard();
        //初始化第一个兵蚁
        auto ant = std::make_shared<Ant>();
        std::cout<<"Please input Piece Ant:\n";
        std::cin>>x>>y;
        std::cout<<std::endl;
        C.q = x;
        C.r = y;
        board.addPiece(ant,C);
        board.printBoard();
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
    // 运动棋子
    while (pos) {
        //输入新位置
        std::cout<<"Please input NewPosition:";
        std::cin>>x>>y;
        newposition.q = x;
        newposition.r = y;
        //移动
        ant->move(board,newposition);
        board.printBoard();
        //停止程序
        std::cout<<"if you want continue,please type 1\n";
        std::cin>>pos;
        if(!pos) break;
    }


    return 0;
    }
