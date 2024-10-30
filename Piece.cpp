//
// Created by 李世佳 on 24-10-15.
//
#include <algorithm>
#include "Hive.h"

using namespace piecetype;
bool QueenBee::canSlideTo(const HexCoord &newPosition, const Board & board) const {
        HexCoord currentPos = getPosition();

        // 获取当前位置和目标位置之间的共同相邻格子
        auto currentNeighbors = currentPos.neighbors();
        auto targetNeighbors = newPosition.neighbors();
        std::vector<HexCoord> commonNeighbors;

        for (const auto& current : currentNeighbors) {
            for (const auto& target : targetNeighbors) {
                if (current == target) {
                    commonNeighbors.push_back(current);
                }
            }
        }
        // 检查是否至少有一个空的共同相邻格子
        for (const auto& neighbor : commonNeighbors) {
            if (!board.isPositionOccupied(neighbor)) {
                return true;
            }
        }
        return false;
}
bool QueenBee::isValidMove(const HexCoord &newPosition, const Board &board) const {
    // 1. 基本验证
    if (!board.isValidPosition(newPosition)) return false;
    if (position == newPosition) return false;

    // 2. 检查移动距离是否为1
    if (position.distance(newPosition) != 1) return false;

    // 3. 检查是否可以滑动到目标位置
    if (!canSlideTo(newPosition, board)) return false;

    // 4. 检查目标位置是否已被占据
    if (board.isPositionOccupied(newPosition)) return false;

    return true;
}
void QueenBee::move(Board &board, const HexCoord &newPosition,const PlayerID&currentPlayer) {
    // 1. 验证移动者身份
    if (ID != currentPlayer) {
        throw InvalidMoveException("Only the owner can move this piece");
    }

    // 2. 验证移动的合法性
    if (!isValidMove(newPosition, board)) {
        throw InvalidMoveException("Invalid queen bee move");
    }

    // 3. 尝试移动并检查蜂巢连续性
    HexCoord oldPosition = getPosition();
    auto piece = board.removePiece(oldPosition);

    // 临时移除后检查连续性
    if (!board.isHiveContinuous()) {
        board.addPiece(piece, oldPosition, ID);
        throw HiveContinuityException("Ant is't contiuity");
    }

    // 4. 执行移动
    setPosition(newPosition);
    board.addPiece(piece, newPosition, ID);
    board.setqueenBeePositions(newPosition, ID);
    recordMove(newPosition);
}
bool Ant::isValidMove(const HexCoord &newPosition, const Board &board) const {
    // 检查新位置是否有效
    if (!board.isValidPosition(newPosition)) {

        return false;

    }
    // 检查新位置是否已被占据
    if (!board.isPositionOccupied(newPosition)) {
        return false;
    }
    // 获取新位置的所有邻居
    std::vector<HexCoord> neighbors = newPosition.neighbors();

    // 检查至少有一个邻居位置被占据
    for (const auto& neighbor : neighbors) {
        if (!board.isPositionOccupied(neighbor)) {
            return true;
        }
    }
    // 如果没有邻居位置被占据，则移动无效
    return false;
}
void Ant::move(Board &board, const HexCoord &newPosition, const PlayerID &currentPlayer) {
    // 1. 验证移动者身份
    if (ID != currentPlayer) {
        throw InvalidMoveException("Only the owner can move this piece");
    }

    // 2. 验证移动的合法性
    if (!isValidMove(newPosition, board)) {
        throw InvalidMoveException("Invalid ant move");
    }

    // 3. 检查目标位置是否已被占据
    if (!board.isPositionOccupied(newPosition)) {
        throw InvalidMoveException("The target position must be occupied by another piece");
    }

    // 4. 尝试移动并检查蜂巢连续性
    HexCoord oldPosition = getPosition();
    std::cout<<"Position input in fonction is["<<oldPosition.q<<","<<oldPosition.r<<"]"<<std::endl;
    board.printPieceInfoAt(oldPosition);
    auto piece = board.removePiece(oldPosition);
    board.printPieceInfoAt(oldPosition);
    // 临时移除后检查连续性
    if (!board.isHiveContinuous()) {
        board.addPiece(piece, oldPosition, ID);
        throw HiveContinuityException("Ant is't contiuity");
    }

    // 5. 执行移动
    setPosition(newPosition);
    board.printPieceInfoAt(getPosition());
    board.addPiece(piece, newPosition, ID);
    board.printPieceInfoAt(newPosition);
    recordMove(newPosition);
}
bool Spider::isValidMove(const HexCoord &newPosition, const Board &board) const {

}
void Spider::move(Board &board, const HexCoord &newPosition, const PlayerID &) {

}




HexCoord QueenBee::getPosition() const {
    return position;
}
HexCoord Ant::getPosition() const {
    return position;
}
HexCoord Spider::getPosition() const {
    return  position;
}






