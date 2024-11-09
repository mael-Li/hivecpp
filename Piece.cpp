//
// Created by 李世佳 on 24-10-15.
//
#include <algorithm>
#include <queue>
#include <set>

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
    if (board.isPositionOccupied(newPosition)) {
        return false;
    }
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

    // 5. 执行移动
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
    // 检查是否是"眼"位置
    if (board.isEye(newPosition)) {
        throw Pieceexception("Eye");
        return false; // 蚂蚁不能移动到"眼"位置
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

    // 4. 尝试移动并检查蜂巢连续性
    HexCoord oldPosition = getPosition();
    std::cout<<"Position input in fonction is["<<oldPosition.q<<","<<oldPosition.r<<"]"<<std::endl;
    auto piece = board.removePiece(oldPosition);

    // 5. 执行移动
    setPosition(newPosition);
    board.addPiece(piece, newPosition, ID);
    recordMove(newPosition);
}
bool Spider::isValidMove(const HexCoord &newPosition, const Board &board) const {
    // 1. 基本检查
    if (!board.isValidPosition(newPosition) || board.isPositionOccupied(newPosition)) {
        return false;
    }

    // 2. 计算距离
    int dx = newPosition.q - position.q;
    int dy = newPosition.r - position.r;
    int distance = std::max({std::abs(dx), std::abs(dy), std::abs(dx + dy)});

    // 距离必须为3
    if (distance != 3) {
        return false;
    }

    // 3. 检查目标位置是否至少与一个其他棋子相邻
    bool hasAdjacentPiece = false;
    for (const HexCoord& neighbor : newPosition.neighbors()) {
        if (neighbor != position && board.isPositionOccupied(neighbor)) {
            hasAdjacentPiece = true;
            break;
        }
    }
    if (!hasAdjacentPiece) {
        return false;
    }

    return true;

    /*
    // 1. 基本验证
    if (!board.isValidPosition(newPosition)) return false;
    if (board.isPositionOccupied(newPosition)) {
        return false;
    }
    if (position == newPosition) return false;
    // 2. 检查移动步数是否精确为3步
    std::vector<HexCoord> path;
    HexCoord currentPos = position;
    HexCoord prevPos = position;

    // 尝试找到一条有效的3步移动路径
    for (int step = 0; step < 3; ++step) {
        bool foundValidStep = false;

        // 获取当前位置的邻居
        auto neighbors = currentPos.neighbors();

        for (const auto& neighbor : neighbors) {
            // 不能原路返回
            if (neighbor == prevPos) continue;
            // 找到一个可以移动的相邻位置
            prevPos = currentPos;
            currentPos = neighbor;
            path.push_back(currentPos);
            foundValidStep = true;
            break;
        }

        // 如果无法找到有效的移动，返回false
        if (!foundValidStep) return false;
    }

    // 最后一步必须是目标位置
    return (currentPos == newPosition);
    */
}
void Spider::move(Board &board, const HexCoord &newPosition, const PlayerID &currentPlayer) {
    // 1. 验证移动者身份
    if (ID != currentPlayer) {
        throw InvalidMoveException("Only the owner can move this piece");
    }

    // 2. 验证移动的合法性
    if (!isValidMove(newPosition, board)) {
        throw InvalidMoveException("Invalid spider move");
    }
    // 3. 尝试移动并检查蜂巢连续性
    HexCoord oldPosition = getPosition();
    auto piece = board.removePiece(oldPosition);

    // 5. 执行移动
    setPosition(newPosition);
    board.addPiece(piece, newPosition, ID);
    recordMove(newPosition);
}

bool Grasshopper::isValidMove(const HexCoord& newPosition, const Board& board) const{
    // 1. 基本验证
    if (!board.isValidPosition(newPosition)) return false;
    if (position == newPosition) return false;
    if (board.isPositionOccupied(newPosition)) {
        return false;
    }
    // 2. 检查跳跃方向是否在同一直线上
    if (position.q == newPosition.q ||
        position.r == newPosition.r ||
        position.q + position.r == newPosition.q + newPosition.r) {
        return true;
        }

    // 3. 检查跳跃路径
    HexCoord currentPos = position;
    HexCoord direction = newPosition - currentPos;
    bool hasJumpedOverPiece = false;

    currentPos += direction;
    while (currentPos != newPosition) {
        if (!board.isPositionOccupied(currentPos)) {
            return false;
        }
        hasJumpedOverPiece = true;
        currentPos += direction;
    }

    return true;
}
void Grasshopper::move(Board &board, const HexCoord &newPosition, const PlayerID &currentPlayer) {
    // 1. 验证移动者身份
    if (ID != currentPlayer) {
        throw InvalidMoveException("Only the owner can move this piece");
    }

    // 2. 验证移动的合法性
    if (!isValidMove(newPosition, board)) {
        throw InvalidMoveException("Invalid grasshopper move");
    }

    // 3. 尝试移动并检查蜂巢连续性
    HexCoord oldPosition = getPosition();
    auto piece = board.removePiece(oldPosition);

    // 5. 执行移动
    setPosition(newPosition);
    board.addPiece(piece, newPosition, ID);
    recordMove(newPosition);
}
// 修改 Beetle 类的移动验证
bool Beetle::isValidMove(const HexCoord &newPosition, const Board &board) const {
    // 1. 基本验证
    if (!board.isValidPosition(newPosition)) return false;
    if (position == newPosition) return false;

    // 2. 检查移动距离是否为1，甲虫只能移动相邻的格子
    if (position.distance(newPosition) > 1) return false;

    // 3. 如果目标位置已被占据，甲虫可以爬上去
    // 不需要额外验证，因为甲虫总是可以爬到其他棋子上

    return true;
}
void Beetle::move(Board &board, const HexCoord &newPosition, const PlayerID &currentPlayer) {
    // 1. 验证移动者身份
    if (ID != currentPlayer) {
        throw InvalidMoveException("Only the owner can move this piece");
    }

    // 2. 验证移动的合法性
    if (!isValidMove(newPosition, board)) {
        throw InvalidMoveException("Invalid beetle move");
    }

    // 3. 尝试移动并检查蜂巢连续性
    HexCoord oldPosition = getPosition();
    auto piece = board.removePiece(oldPosition);

    // 5. 执行移动
    setPosition(newPosition);
    board.addPiece(piece, newPosition, ID);
    recordMove(newPosition);
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
HexCoord Grasshopper::getPosition() const {
    return position;
}
HexCoord Beetle::getPosition() const {
    return position;
}




