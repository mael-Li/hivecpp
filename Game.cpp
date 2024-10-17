//
// Created by 李世佳 on 24-10-17.
//

#include "Game.h"
using namespace piecetype;

Game::Game() : board(5) {
    // 初始化玩家
    players.push_back(std::make_shared<HumanPlayer>("Human"));
    players.push_back(std::make_shared<AIPlayer>("AI"));
}
void Game::start() {
    // 游戏主循环
    while (true) {
        for (auto player : players) {
            std::cout << "It's " << player->getName() << "'s turn." << std::endl;
            player->makeMove(board);
            std::cout << "Current board state:" << std::endl;
            board.printBoard();
        }
    }
}

HumanPlayer::HumanPlayer(std::string n){setName(n);}

void HumanPlayer::makeMove(piecetype::Board& board) {
    std::cout << getName() << ", enter your move (e.g., 'place piece at x y' or 'move fromX fromY toX toY'): ";
    std::string command;
    std::cin >> command;

    if (command == "place") {
        std::string pieceType;
        int x, y;
        std::cin >> pieceType >> x >> y;
        // 创建棋子实例
        std::shared_ptr<Piece> piece;
        if (pieceType == "Q") {
            piece = std::make_shared<QueenBee>();
        } else if (pieceType == "A") {
            piece = std::make_shared<Ant>();
        } else {
            std::cout << "Unknown piece type." << std::endl;
            return;
        }
        // 放置棋子
        board.addPiece(piece, HexCoord(x, y));
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
AIPlayer::AIPlayer(std::string name){setName(name);}

void AIPlayer::makeMove(piecetype::Board& board) {
    // AI 的逻辑来决定移动
    std::cout << getName() << " is making a move..." << std::endl;
    // 示例移动逻辑
    auto pieces = board.getAllPiecesOnBoard(board.getSize());
    if (!pieces.empty()) {
        auto piece = pieces.front();
        piece->move(board, HexCoord(1, 1)); // 示例移动
    }
}