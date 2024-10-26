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
void QueenBee::move(Board &board, const HexCoord &newPosition,const PlayerID&) {
    if (isValidMove(newPosition,board)) {
        setPosition(newPosition);
        board.addPiece(shared_from_this(), newPosition,ID);
    } else {
        throw Pieceexception("Queen can't move");
    }
}
bool Ant::isValidMove(const HexCoord &newPosition, const Board &board) const {
    //对于蚂蚁类的移动应该满足这些条件，可以沿着棋子移动
    auto neighbers = newPosition.neighbors();
    if(neighbers.empty()) return false;
    return board.ishasNeighber(newPosition);
}
void Ant::move(Board &board, const HexCoord &newPosition,const PlayerID&i) {
    auto currentPiece = board.getPieceAt(position);
    if (currentPiece && currentPiece->getID() != i) {
        throw Pieceexception("Only the owner of the piece can move it.");
    }
    if(isValidMove(newPosition,board)) {
        HexCoord oldposition = getPosition();
        setPosition(newPosition);
        board.removePiece(oldposition);
        board.addPiece(shared_from_this(),newPosition, ID);
    }
    else {
        throw Pieceexception("Invalid move position");
    }
}
HexCoord Ant::getPosition() const {
    return position;
}



