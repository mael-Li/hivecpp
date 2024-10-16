//
// Created by 李世佳 on 24-10-15.
//
#include <algorithm>
#include "Hive.h"

using namespace piecetype;
bool QueenBee::isValidMove(const HexCoord &newPosition, const Board &board) const {
    if (position.distance(newPosition) > 1) {
        return false;
    }
    return !board.isPositionOccupied(newPosition);
}
void QueenBee::move(Board &board, const HexCoord &newPosition) {
    if (isValidMove(newPosition,board)) {
        setPosition(newPosition);
        board.addPiece(shared_from_this(), newPosition);
    } else {
        std::cerr << "Invalid move for Queen from (" << position.q << ", " << position.r
                  << ") to (" << newPosition.q << ", " << newPosition.r << ")" << std::endl;
    }
}


