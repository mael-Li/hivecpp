//
// Created by 李世佳 on 24-10-17.
//
#include "Game.h"
using namespace piecetype;
int getMenuChoice() {
    std::vector<std::string> options = {"Place Piece", "Move Piece", "Exit"};
    int choice = -1;

    while (choice != 2) { // 2对应"Move Piece"
        std::cout << "\nHive Game Menu:\n";
        for (size_t i = 0; i < options.size(); ++i) {
            std::cout << i + 1 << ". " << options[i] << "\n";
        }
        std::cout << "Choose an option: ";

        if (!(std::cin >> choice) || choice < 1 || choice > static_cast<int>(options.size())) {
            std::cout << "Invalid input, please try again.\n";
            std::cin.clear(); // Reset failbit
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore rest of line
            continue;
        }

        switch (choice) {
            case 1: // Place Piece
                std::cout << "Placing a piece...\n";
            // Add logic for placing a piece
            return 1;
            case 2: // Move Piece
                std::cout << "Moving a piece...\n";
            // Add logic for moving a piece
            return 2;
            case 3: // Exit
                std::cout << "Exiting the game...\n";
            return 0;
            default:
                std::cout << "Unsupported option.\n";
        }
    }
    return 0;
}
Game::Game() : board(5) {
    // 初始化玩家
    players.push_back(std::make_shared<HumanPlayer>("Human",PlayerId::player1));
    players.push_back(std::make_shared<HumanPlayer>("Human",PlayerId::player2));
}
void Game::start() {
    // 游戏主循环
    while (true) {
        for (auto player : players) {
            std::cout << "It's " << player->getName() << "'s turn." << std::endl;
            // 显示菜单并获取用户的选择
            int commandorder = getMenuChoice();
            player->makeMove(board,commandorder);
            std::cout << "Current board state:" << std::endl;
            board.printBoard();
        }
    }
}

HumanPlayer::HumanPlayer(std::string n,PlayerId a){setName(n);setId(a);}

void HumanPlayer::makeMove(piecetype::Board& board,int c) {
    std::string player;
    if(getid() == PlayerId::player1) player ="player1";
    else player = "player2";
    std::cout << player<< ",please enter your type of :";
    std::string command;
    //std::cin >> command;
    if(c == 1) command = "place";
    else if(c == 2) command = "move";
//放置棋子的逻辑
    if (command == "place") {
        std::string pieceType;
        std::cin >> pieceType;
        int x, y;
        std::cout<<"please enter your position of piece:";
        std::cin>>x>>y;
        std::cout<<std::endl;
        // 创建棋子实例
        std::shared_ptr<Piece> piece;
        if (pieceType == "Q") {
            piece = std::make_shared<QueenBee>(getid());
        } else if (pieceType == "A") {
            piece = std::make_shared<Ant>();
        } else {
            std::cout << "Unknown piece type." << std::endl;
            return;
        }
        // 放置棋子
        board.addPiece(piece, HexCoord(x, y),getid());
//移动棋子的逻辑
    } else if (command == "move") {
        int fromX, fromY, toX, toY;
        std::cin >> fromX >> fromY >> toX >> toY;
        // 获取棋子
        auto piece = board.getPieceAt(HexCoord(fromX, fromY));
        if (piece) {
            // 移动棋子
            piece->move(board, HexCoord(toX, toY));
        } else {
            std::cout << "No piece at the given coordinate." << std::endl;
        }
    } else {
        std::cout << "Invalid command." << std::endl;
    }
}
/*
AIPlayer::AIPlayer(std::string n){setName(n);}

void AIPlayer::makeMove(piecetype::Board& board,int) {
    // AI 的逻辑来决定移动
    std::cout << getName() << " is making a move..." << std::endl;
    // 示例移动逻辑
    auto queen = std::make_shared<QueenBee>();
    board.addPiece(queen,HexCoord(1,1));
//
    auto pieces = board.getAllPiecesOnBoard(board.getSize());
    if (!pieces.empty()) {
        auto piece = pieces.front();
        piece->move(board, HexCoord(1, 1)); // 示例移动
    }
//
}
    */