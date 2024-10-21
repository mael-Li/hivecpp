//
// Created by 李世佳 on 24-10-14.
//
#include <cassert>

#include"Hive.h"
using namespace piecetype;

void Board::initializePiecesAvailable() {
    // 设置每种棋子的最大数量
    piecesAvailable[PlayerID::player1][PieceName::Queen] = 1;
    piecesAvailable[PlayerID::player1][PieceName::Ant] = 7;
    //piecesAvailable[PlayerID::player1][PieceName:] = 1;
    //piecesAvailable[PlayerID::player1][PieceName:] = 1;
    //piecesAvailable[PlayerID::player1][PieceName:] = 1;
    //piecesAvailable[PlayerID::player1][PieceName:] = 1;

    piecesAvailable[PlayerID::player2][PieceName::Queen] = 1;
    piecesAvailable[PlayerID::player2][PieceName::Ant] = 7;
    //piecesAvailable[PlayerID::player2][PieceName:] = 1;
    //piecesAvailable[PlayerID::player2][PieceName:] = 1;
    //piecesAvailable[PlayerID::player2][PieceName:] = 1;
    //piecesAvailable[PlayerID::player2][PieceName:] = 1;
}



void Board::addPiece(std::shared_ptr<Piece> piece, HexCoord coord, PlayerID player) {
    if (piecesAvailable[player][piece->getEumName()] <= 0) {
        throw std::runtime_error("No more pieces of this type available.");
    }
    grid[coord] = piece;
    piece->setPosition(coord);
    piece->setID(player);
    piecesAvailable[player][piece->getEumName()]--; // 减少可用的棋子数量
}
void Board::removePiece(HexCoord coord) {
    auto it = grid.find(coord);
    if(it != grid.end()) {
        grid.erase(coord);
    }else {
        std::cerr << "No piece at the given coordinate." << std::endl;
    }
}
std::shared_ptr<Piece> Board::getPieceAt(HexCoord coord) const {
    auto it = grid.find(coord);
    if (it != grid.end()) {
        return it->second;
    }
    return nullptr;
}
bool Board::isValidPosition(HexCoord coord) const {
    return abs(coord.q)<size&&abs(coord.r)<size;
}
bool Board::isPositionOccupied(HexCoord coord) const {
    return grid.find(coord) != grid.end();
}
bool Board::ishasNeighber(HexCoord coord) const {
    std::vector<HexCoord> neighbors = coord.neighbors();
    for (const auto& neighbor : neighbors) {
        if (isValidPosition(neighbor) && isPositionOccupied(neighbor)) {
            return true;
        }
    }
    return false;
}
bool Board::isQueenBeeSurround(PlayerID player) const {
    const HexCoord& queenBeePos = queenBeePositions.at(player);
    std::vector<HexCoord> neighbors = queenBeePos.neighbors();
    // 检查蜂后周围的六个位置是否全部被占据
    for (const auto& neighbor : neighbors) {
        if (!isPositionOccupied(neighbor)) {
            return false; // 至少有一个空位，蜂后可以移动
        }
    }
    return true; // 所有位置都被占据，蜂后无法移动
}
PlayerID Board::checkVictory() const {
    if (isQueenBeeSurround(PlayerID::player1)) {
        return PlayerID::player1;
    }
    if (isQueenBeeSurround(PlayerID::player2)) {
        return PlayerID::player2;
    }
    return PlayerID::playernobody; // 无胜利者，可以返回任意玩家
}

std::vector<std::shared_ptr<Piece>> Board::getAllPiecesOnBoard(int size)const {
    std::vector<std::shared_ptr<Piece>> pieces;
    // 获取棋盘的最大尺寸
    int maxSize = getSize();
    if(size>maxSize) size = maxSize;
    // 遍历所有可能的坐标
    for (int q = -maxSize; q <= maxSize; ++q) {
        for (int r = -maxSize; r <= maxSize; ++r) {
            HexCoord coord(q, r);
            // 检查坐标是否有效
            if (isValidPosition(coord)) {
                // 获取该位置上的棋子
                std::shared_ptr<Piece> piece = getPieceAt(coord);
                if (piece) {
                    pieces.push_back(piece);
                }
            }
        }
    }
    return pieces;
}


void Board::printBoard() const {
    std::cout<<"Board["<<index<<"]"<<std::endl;
    std::string player;
    for (int row =-size;row<=size;++row) {
        for (int col = -size;col<size;++col) {
            HexCoord coord(row, col);
            if(isValidPosition(coord)) {
                std::shared_ptr<Piece>piece = getPieceAt(coord);
                if (piece) {
                    if(piece->getID() == PlayerID::player1) player = "1";
                    else player = "2";
                    std::cout << "["<< piece->getName();
                    std::cout<<player<<"]";
                }else {
                    std::cout << "[  ]";
                }
            }
        }
        std::cout << std::endl;
    }
}