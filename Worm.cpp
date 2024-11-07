//
// Created by 李世佳 on 24-11-7.
//
#include "Hive.h"
namespace piecetype {
    bool Worm::isValidMove(const HexCoord& newPosition, const Board& board) const {
        // 基本验证
        if (!board.isValidPosition(newPosition)) return false;
        if (position == newPosition) return false;

        // 检查移动距离是否为3
        if (position.distance(newPosition) != 3) return false;

        // 检查目标位置是否有棋子（需要有棋子才能交换）
        if (!board.isPositionOccupied(newPosition)) return false;

        // 检查路径上是否有连续的棋子（模拟蚯蚓在蜂巢下移动）
        HexCoord direction = HexCoord(
            (newPosition.q - position.q) / 3,
            (newPosition.r - position.r) / 3
        );

        HexCoord current = position;
        for (int i = 0; i < 2; ++i) {  // 检查路径上的两个中间点
            current = HexCoord(current.q + direction.q, current.r + direction.r);
            if (!board.isPositionOccupied(current)) {
                return false;  // 路径必须是连续的
            }
        }

        return true;
    }

    void Worm::move(Board& board, const HexCoord& newPosition, const PlayerID& currentPlayer) {
        // 验证移动者身份
        if (ID != currentPlayer) {
            throw InvalidMoveException("Only the owner can move this piece");
        }

        // 验证移动的合法性
        if (!isValidMove(newPosition, board)) {
            throw InvalidMoveException("Invalid worm move");
        }

        // 获取目标位置的棋子（用于交换）
        auto targetPiece = board.getTopPiece(newPosition);
        if (!targetPiece) {
            throw InvalidMoveException("No piece to swap with");
        }

        // 保存当前位置
        HexCoord oldPosition = getPosition();

        // 移除两个位置的棋子
        auto wormPiece = board.removePiece(oldPosition);
        targetPiece = board.removePiece(newPosition);

        // 检查移动后的连续性
        if (!board.isHiveContinuous()) {
            // 如果移动会破坏连续性，恢复原状
            board.addPiece(wormPiece, oldPosition, ID);
            board.addPiece(targetPiece, newPosition, targetPiece->getID());
            throw HiveContinuityException("Worm move would break hive continuity");
        }

        // 交换位置
        setPosition(newPosition);
        targetPiece->setPosition(oldPosition);

        // 将棋子放置在新位置
        board.addPiece(wormPiece, newPosition, ID);
        board.addPiece(targetPiece, oldPosition, targetPiece->getID());

        recordMove(newPosition);
    }
}