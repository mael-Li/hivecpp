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
        throw Pieceexception("Ant can't move");
    }
}
HexCoord Ant::getPosition() const {
    return position;
}
HexCoord Spider::getPosition() const {
    return position;
}
bool Spider::isValidMove(const HexCoord &newPosition, const Board &board) const {
    auto neighbors = newPosition.neighbors();
    if (neighbors.empty()) return false;

    HexCoord oldPosition = getPosition();
    for (const auto& neighbor : neighbors) {
        if (oldPosition == neighbor) continue; // 跳过原位置
        if (board.isPositionOccupied(neighbor) && board.getPieceAt(neighbor)->getID() == ID) {
            // 如果相邻位置有同属于当前玩家的棋子，则认为是有效移动
            return true;
        }
    }
    return false;
}
void Spider::move(Board &board, const HexCoord &newPosition, const PlayerID &i) {
    auto currentPiece = board.getPieceAt(position);
    if (currentPiece && currentPiece->getID() != i) {
        throw Pieceexception("Only the owner of the piece can move it.");
    }
    if (isValidMove(newPosition, board)) {
        setPosition(newPosition);
        board.addPiece(shared_from_this(), newPosition, ID);
    } else {
        throw Pieceexception("Spider can't move");
    }
}
bool Grasshopper::isValidMove(const HexCoord &newPosition, const Board &board) const {
    // 实现逻辑判断是否可以跳过一个棋子到达新位置
    // 返回true或false
    return board.isValidPosition(newPosition) && !board.isPositionOccupied(newPosition);
}
void Grasshopper::move(Board &board, const HexCoord &newPosition, const PlayerID &i) {
    auto currentPiece = board.getPieceAt(position);
    if (currentPiece && currentPiece->getID() != i) {
        throw Pieceexception("Only the owner of the piece can move it.");
    }
    if (isValidMove(newPosition, board)) {
        setPosition(newPosition);
        board.addPiece(shared_from_this(), newPosition, ID);
    } else {
        throw Pieceexception("Grasshopper can't move");
    }
}
bool Beetle::isValidMove(const HexCoord &newPosition, const Board &board) const {
    auto neighbors = newPosition.neighbors();
    for (auto &neighbor : neighbors) {
        if (neighbor == position) {
            return true;
        }
    }
    return false;
}
void Beetle::move(Board &board, const HexCoord &newPosition, const PlayerID &i) {
    auto currentPiece = board.getPieceAt(position);
    if (currentPiece && currentPiece->getID() != i) {
        throw Pieceexception("Only the owner of the piece can move it.");
    }
    if (isValidMove(newPosition, board)) {
        setPosition(newPosition);
        board.addPiece(shared_from_this(), newPosition, ID);
    } else {
        throw Pieceexception("Beetle can't move");
    }
}





