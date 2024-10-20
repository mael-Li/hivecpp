//
// Created by 李世佳 on 24-10-14.
//
#include"Hive.h"
using namespace piecetype;


void Board::addPiece(std::shared_ptr<Piece> piece, HexCoord coord, PlayerID player) {
    grid[coord] = piece;
    piece->setPosition(coord);
    piece->setID(player);
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