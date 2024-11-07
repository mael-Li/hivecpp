//
// Created by 李世佳 on 24-11-7.
//
#include "Game.h"
namespace piecetype {

    void AIPlayer::makeMove(Board& board, GameCommand command) {
        switch (command) {
            case GameCommand::PLACE_PIECE:
                handleAIPlacePiece(board);
                break;
            case GameCommand::MOVE_PIECE:
                handleAIMovePiece(board);
                break;
            default:
                throw std::runtime_error("Invalid command for AI player");
        }
        incrementMoveCount();
    }

    void AIPlayer::handleAIPlacePiece(Board& board) {
        // 检查是否需要放置蜂后
        bool queenRequired = (board.getTurnCount() >= 6 && !board.isQueenPlaced(getID()));

        if (queenRequired) {
            // 寻找安全的蜂后位置
            HexCoord pos = findBestPlacement(board, PieceName::Queen);
            auto queen = createPiece(PieceName::Queen);
            board.addPiece(queen, pos, getID());
            std::cout << getName() << " places Queen at (" << pos.q << "," << pos.r << ")\n";
            return;
        }

        // 选择要放置的棋子类型
        PieceName selectedType = PieceName::Queen; // 默认值
        int bestScore = -1;

        // 评估每种可用棋子
        for (const auto& [type, count] : board.piecesAvailable[getID()]) {
            if (count > 0) {
                // 找到这种棋子的最佳位置
                HexCoord bestPos = findBestPlacement(board, type);
                int score = evaluatePosition(board, bestPos, type);
                if (score > bestScore) {
                    bestScore = score;
                    selectedType = type;
                }
            }
        }

        // 放置选中的棋子
        HexCoord pos = findBestPlacement(board, selectedType);
        auto piece = createPiece(selectedType);
        board.addPiece(piece, pos, getID());

        // 输出AI的决策
        std::string pieceName;
        switch (selectedType) {
            case PieceName::Queen: pieceName = "Queen"; break;
            case PieceName::Ant: pieceName = "Ant"; break;
            case PieceName::Spider: pieceName = "Spider"; break;
            case PieceName::Beetle: pieceName = "Beetle"; break;
            case PieceName::Grasshopper: pieceName = "Grasshopper"; break;
            case PieceName::Worm: pieceName = "Worm"; break;
        }
        std::cout << getName() << " places " << pieceName << " at (" << pos.q << "," << pos.r << ")\n";
    }

    void AIPlayer::handleAIMovePiece(Board& board) {
        if (!board.isQueenPlaced(getID())) {
            throw InvalidMoveException("AI must place Queen Bee before moving pieces!");
        }

        // 找到最佳移动
        auto [from, to] = findBestMove(board);
        auto piece = board.getPieceAt(from);
        if (!piece) {
            throw InvalidMoveException("No piece at selected position");
        }

        // 执行移动
        piece->move(board, to, getID());
        std::cout << getName() << " moves piece from (" << from.q << "," << from.r
                  << ") to (" << to.q << "," << to.r << ")\n";
    }

    std::shared_ptr<Piece> AIPlayer::createPiece(const PieceName& type) {
        switch (type) {
            case PieceName::Queen: return std::make_shared<QueenBee>(id);
            case PieceName::Ant: return std::make_shared<Ant>(id);
            case PieceName::Spider: return std::make_shared<Spider>(id);
            case PieceName::Beetle: return std::make_shared<Beetle>(id);
            case PieceName::Grasshopper: return std::make_shared<Grasshopper>(id);
            case PieceName::Worm: return std::make_shared<Worm>(id);
            default: return nullptr;
        }
    }

    HexCoord AIPlayer::findBestPlacement(Board& board, PieceName pieceType) {
        HexCoord bestPos(0, 0);
        int bestScore = -1000;

        // 遍历棋盘所有可能的位置
        for (int q = -board.getSize(); q <= board.getSize(); q++) {
            for (int r = -board.getSize(); r <= board.getSize(); r++) {
                HexCoord pos(q, r);
                if (board.isValidPosition(pos) && !board.isPositionOccupied(pos) &&
                    board.canPlacePiece(pos, getID())) {
                    int score = evaluatePosition(board, pos, pieceType);
                    if (score > bestScore) {
                        bestScore = score;
                        bestPos = pos;
                    }
                }
            }
        }

        return bestPos;
    }

    std::pair<HexCoord, HexCoord> AIPlayer::findBestMove(Board& board) {
        HexCoord bestFrom(0, 0), bestTo(0, 0);
        int bestScore = -1000;

        // 遍历所有我方棋子
        for (int q = -board.getSize(); q <= board.getSize(); q++) {
            for (int r = -board.getSize(); r <= board.getSize(); r++) {
                HexCoord from(q, r);
                if (!board.isValidPosition(from)) continue;

                auto piece = board.getTopPiece(from);
                if (!piece || piece->getID() != getID()) continue;

                // 获取所有可能的移动位置
                auto possibleMoves = board.getPossibleMoves(from);
                for (const auto& to : possibleMoves) {
                    int score = evaluateMove(board, from, to);
                    if (score > bestScore) {
                        bestScore = score;
                        bestFrom = from;
                        bestTo = to;
                    }
                }
            }
        }

        return {bestFrom, bestTo};
    }

    int AIPlayer::evaluatePosition(const Board& board, const HexCoord& pos, PieceName pieceType) const {
        int score = 0;

        // 基础分数
        if (isPositionSafe(board, pos)) score += 10;
        if (isPositionStrategic(board, pos)) score += 15;

        // 根据棋子类型调整分数
        switch (pieceType) {
            case PieceName::Queen:
                // 蜂后优先选择安全且不易被围住的位置
                if (board.getPossibleMoves(pos).size() >= 3) score += 20;
                break;
            case PieceName::Beetle:
                // 甲虫优先选择靠近敌方蜂后的位置
                if (calculateDistanceToEnemyQueen(board, pos) <= 2) score += 25;
                break;
            case PieceName::Ant:
                // 蚂蚁优先选择可以快速移动到关键位置的位置
                if (board.getPossibleMoves(pos).size() > 4) score += 15;
                break;
            // 其他棋子的评分规则...
        }

        return score;
    }

    int AIPlayer::evaluateMove(const Board& board, const HexCoord& from, const HexCoord& to) const {
        int score = 0;

        // 评估移动后的位置
        auto piece = board.getTopPiece(from);
        if (!piece) return -1000;

        // 如果这是蜂后
        if (piece->getEumName() == PieceName::Queen) {
            // 优先远离危险
            if (isPositionSafe(board, to)) score += 30;
            if (board.getPossibleMoves(to).size() > board.getPossibleMoves(from).size()) score += 15;
        }

        // 如果可以围住对手的蜂后
        PlayerID enemyId = (getID() == PlayerID::player1) ? PlayerID::player2 : PlayerID::player1;
        if (board.isQueenPlaced(enemyId)) {
            // 如果移动后能增加对敌方蜂后的围困程度
            // 这里需要补充具体的围困检查逻辑
        }

        return score;
    }

    bool AIPlayer::isPositionSafe(const Board& board, const HexCoord& pos) const {
        // 检查周围是否有敌方棋子
        auto neighbors = pos.neighbors();
        int enemyCount = 0;
        for (const auto& neighbor : neighbors) {
            if (board.isValidPosition(neighbor)) {
                auto piece = board.getTopPiece(neighbor);
                if (piece && piece->getID() != getID()) {
                    enemyCount++;
                }
            }
        }
        return enemyCount <= 1; // 如果周围敌方棋子不超过1个，认为是安全的
    }

    bool AIPlayer::isPositionStrategic(const Board& board, const HexCoord& pos) const {
        // 位置是否有战略价值（例如：靠近敌方蜂后、控制关键通道等）
        PlayerID enemyId = (getID() == PlayerID::player1) ? PlayerID::player2 : PlayerID::player1;
        if (!board.isQueenPlaced(enemyId)) return false;

        // 这里可以添加更多战略位置的判断逻辑
        return calculateDistanceToEnemyQueen(board, pos) <= 2;
    }

    int AIPlayer::calculateDistanceToEnemyQueen(const Board& board, const HexCoord& pos) const {
        PlayerID enemyId = (getID() == PlayerID::player1) ? PlayerID::player2 : PlayerID::player1;
        if (!board.isQueenPlaced(enemyId)) return 1000;

        // 获取敌方蜂后位置并计算距离
        auto queenIt = board.getqueenBeePositions().find(enemyId);
        if (queenIt != board.getqueenBeePositions().end()) {
            return pos.distance(queenIt->second);
        }
        return 1000;
    }
}