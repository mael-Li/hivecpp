//
// Created by 李世佳 on 24-10-14.
//
#include"Hive.h"
using namespace piecetype;


void Board::addPiece(std::shared_ptr<Piece> piece, HexCoord coord) {
    grid[coord] = piece;
}
std::shared_ptr<Piece> Board::getPieceAt(HexCoord coord) const {
    auto it = grid.find(coord);
    if (it != grid.end()) {
        return it->second;
    }
    return nullptr;
}
bool Board::isValidPosition(HexCoord coord) const {
    // Implement check for valid hexagonal grid coordinates
    return abs(coord.q)<size&&abs(coord.r)<size; // Placeholder implementation
}
bool Board::isPositionOccupied(HexCoord coord) const {
    return grid.find(coord) != grid.end();
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
    int index = 0;
    std::cout<<"Board["<<index<<"]"<<std::endl;
    for (int row =-size;row<=size;++row) {
        for (int col = 0;col<size;++col) {
            HexCoord coord(row, col);
            if(isValidPosition(coord)) {
                std::shared_ptr<Piece>piece = getPieceAt(coord);
                if (piece) {
                    piece->getName();
                    std::cout << "["<< piece->getName()<<"]";
                }else {
                    std::cout << "[ ]";
                }
                }
            }
        std::cout << std::endl;
        }
    index++;
}




/*
Board::Board(int size) : size(size) {
    initializeGrid();
}

Board::~Board() {
    // 如果需要手动管理内存，这里可以释放
}
void Board::addPiece(std::shared_ptr<piecetype::Piece> piece, HexCoord coord) {
    int rowIndex = coord.q + size - 1;
    int colIndex = coord.r;

    if (rowIndex >= 0 && rowIndex < grid.size() && colIndex >= 0 && colIndex < grid[rowIndex].size()) {
        grid[rowIndex][colIndex].setPiece(piece);
    } else {
        throw std::out_of_range("Invalid position");
    }
}
std::shared_ptr<piecetype::Piece> Board::getPieceAt(HexCoord coord) const {
    int rowIndex = coord.q + size - 1;
    int colIndex = coord.r;

    if (rowIndex >= 0 && rowIndex < grid.size() && colIndex >= 0 && colIndex < grid[rowIndex].size()) {
        return grid[rowIndex][colIndex].getPiece();
    }
    return nullptr;
}
void Board::printBoard() const {
    for (int row = 0; row < grid.size(); ++row) {
        for (int col = 0; col < grid[row].size(); ++col) {
            const auto& hex = grid[row][col];
            if (hex.getPiece()) {
                std::cout << hex.getPiece()->getName() << ' ';
            } else {
                std::cout << ". ";
            }
        }
        std::cout << std::endl;
    }
}
bool Board::isValidPosition(HexCoord coord) const {
    int rowIndex = coord.q + size - 1;
    int colIndex = coord.r;

    return rowIndex >= 0 && rowIndex < grid.size() && colIndex >= 0 && colIndex < grid[rowIndex].size();
}
bool Board::isPositionOccupied(HexCoord coord) const {
    return getPieceAt(coord) != nullptr;
}

// 实现棋盘类的成员函数
void Board::initializeGrid() {
    //分配大小
    grid.resize(size * 2 - 1);
//棋盘形状
    for (int row = 0; row < grid.size(); ++row){
        int rowSize = std::max(0, size - std::abs(row - size + 1));
        grid[row].resize(rowSize);

        for (int col = 0; col < rowSize; ++col) {
            grid[row][col] = Hexagon();
        }
    }
}
*/