//
// Created by 李世佳 on 24-10-14.
//
#include <cassert>

#include"Hive.h"
using namespace piecetype;

void Board::initializePiecesAvailable() {
    // 设置每种棋子的最大数量
    piecesAvailable[PlayerID::player1][PieceName::Queen] = 1; //蜂后，只能移动一个格子
    piecesAvailable[PlayerID::player1][PieceName::Ant] = 3;//只能移动到周围有邻居的地方
    piecesAvailable[PlayerID::player1][PieceName::Beetle] = 2;//可以压住别人
    piecesAvailable[PlayerID::player1][PieceName::Grasshopper] = 3;//只能移动到棋子的对面
    piecesAvailable[PlayerID::player1][PieceName::Spider] = 2;//不能多不能少，它必须向前移动，不能原路返回。在它移动的每一步，它只能走过有与其直接相连棋子的相邻格子，而不能走到不是与其相连棋子的格子。
    //player2的棋子个数
    piecesAvailable[PlayerID::player2][PieceName::Queen] = 1;
    piecesAvailable[PlayerID::player2][PieceName::Ant] = 3;
    piecesAvailable[PlayerID::player2][PieceName::Beetle] = 2;
    piecesAvailable[PlayerID::player2][PieceName::Grasshopper] = 3;
    piecesAvailable[PlayerID::player2][PieceName::Spider] = 2;
}
void Board::addPiece(std::shared_ptr<Piece> piece, HexCoord coord, PlayerID player) {
    if (!piece) {
        throw std::invalid_argument("Cannot add null piece");
    }

    if (piece->getID() != player) {
        throw std::invalid_argument("Piece does not belong to the current player");
    }

    if (!isValidPosition(coord)) {
        throw std::invalid_argument("Invalid coordinate position");
    }

    // 验证放置位置的合法性
    if (!canPlacePiece(coord, player)) {
        throw InvalidMoveException("Invalid placement position");
    }

    // 创建或获取格子
    if (grid.find(coord) == grid.end()) {
        grid[coord] = Hexagon();
    }

    // 更新棋子数量
    if (piecesAvailable[player][piece->getEumName()] <= 0) {
        throw std::runtime_error("No more pieces of this type available");
    }
    piecesAvailable[player][piece->getEumName()]--;

    // 添加棋子到格子
    piece->setPosition(coord);
    grid[coord].pieces.push_back(piece);

    // 更新首次放置状态
    if (!firstPiecePlaced) {
        firstPiecePlaced = true;
    }

    // 如果是蜂后，更新位置
    if (piece->getEumName() == PieceName::Queen) {
        setqueenBeePositions(coord, player);
    }
}
std::shared_ptr<Piece> Board::removePiece(HexCoord coord) {
    // 1. 检查坐标是否存在于网格中
    auto gridIt = grid.find(coord);
    if (gridIt == grid.end()) {
        throw std::invalid_argument("Coordinate does not exist on board");
    }

    // 2. 检查该位置是否有棋子
    auto& hexagon = gridIt->second;
    if (hexagon.pieces.empty()) {
        throw std::invalid_argument("No piece at this position to remove");
    }

    // 3. 获取要移除的棋子
    auto piece = hexagon.pieces.back();
    if (!piece) {
        throw std::runtime_error("Invalid piece state detected");
    }

    // 4. 移除棋子
    hexagon.pieces.pop_back();

    // 5. 如果是蜂后，清除蜂后位置记录
    if (piece->getEumName() == PieceName::Queen) {
        clearQueenBeePosition(piece->getID());
    }

    // 6. 更新可用棋子数量
    piecesAvailable[piece->getID()][piece->getEumName()]++;

    // 7. 如果格子为空，可以选择从网格中移除该格子
    if (hexagon.pieces.empty()) {
        grid.erase(gridIt);
    }

    // 8. 检查蜂巢连续性
    if (!isHiveContinuous()) {
        // 如果移除导致蜂巢不连续，恢复状态
        if (grid.find(coord) == grid.end()) {
            grid[coord] = Hexagon();
        }
        grid[coord].pieces.push_back(piece);
        piecesAvailable[piece->getID()][piece->getEumName()]--;
        if (piece->getEumName() == PieceName::Queen) {
            setqueenBeePositions(coord, piece->getID());
        }
        throw HiveContinuityException("Removing piece would break hive continuity");
    }

    return piece;
}

std::shared_ptr<Piece> Board::getPieceAt(HexCoord coord) const {
    auto it = grid.find(coord);
    if (it != grid.end() && !it->second.pieces.empty()) {
        return it->second.pieces.back();
    }
    return nullptr;
}
bool Board::isValidPosition(HexCoord coord) const {
    return abs(coord.q)<size&&abs(coord.r)<size;
}
bool Board::isValidMove(const HexCoord &from, const HexCoord &to) const {
    // 检查起始位置是否有棋子
    if (!isPositionOccupied(from)) return false;

    // 检查目标位置是否已被占用（除非是甲虫可以爬上去）
    if (isPositionOccupied(to)) {
        auto piece = getPieceAt(from);
        if (!piece || piece->getEumName() != PieceName::Beetle) {
            return false;
        }
    }

    // 创建一个临时的board状态来验证移动
    return willMoveMaintainContinuity(from, to);
}

bool Board::isPositionOccupied(HexCoord coord) const {
    auto it = grid.find(coord);
    return !(it != grid.end() && !it->second.pieces.empty());
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
    // 尝试获取指定玩家的蜂后位置
    auto it = queenBeePositions.find(player);
    if (it == queenBeePositions.end()) {
        // 如果键不存在，则蜂后位置未知，可以认为蜂后没有被包围
        return false;
    }
    const HexCoord& queenBeePos = it->second;
    std::vector<HexCoord> neighbors = queenBeePos.neighbors();
    // 检查蜂后周围的六个位置是否全部被占据
    for (const auto& neighbor : neighbors) {
        if (!isPositionOccupied(neighbor)) {
            //std::cout<<"The position["<<neighbor.q<<","<<neighbor.r<<"]"<<"is vide"<<std::endl;
            return false; // 至少有一个空位，蜂后可以移动
        }
    }
    return true; // 所有位置都被占据，蜂后无法移动
}
//显示所下的棋子的空位
bool Board::isTopPiece(const HexCoord &coord, PlayerID id) const {
    auto hex = grid.find(coord);
    if(hex != grid.end()&&!hex->second.pieces.empty()) {
        return hex->second.pieces.back()->getID() == id;
    }
    return false;
}
bool Board::canPlacePiece(const HexCoord& coord, PlayerID playerid) const {
    // 第一个棋子可以放在任何位置
    if (!firstPiecePlaced) {
        return true;
    }

    // 获取棋盘上的总棋子数
    int totalPieces = 0;
    for (const auto& [_, hex] : grid) {
        totalPieces += hex.pieces.size();
    }

    // 如果是第二个棋子，只需要检查是否与第一个棋子相邻
    if (totalPieces == 1) {
        auto neighbors = coord.neighbors();
        for (const auto& neighbor : neighbors) {
            if (isPositionOccupied(neighbor)) {
                return true;  // 只要相邻就可以
            }
        }
        return false;  // 没有相邻的棋子
    }

    // 对于后续的棋子，检查是否与友方棋子相邻且不与敌方棋子接触
    bool hasAdjacentFriendly = false;
    auto neighbors = coord.neighbors();

    for (const auto& neighbor : neighbors) {
        if (!isValidPosition(neighbor)) continue;

        if (isPositionOccupied(neighbor)) {
            auto topPiece = getTopPiece(neighbor);
            if (!topPiece) continue;

            if (topPiece->getID() == playerid) {
                hasAdjacentFriendly = true;
            } else if (totalPieces > 1) {  // 第二个棋子不检查敌方接触
                return false;  // 接触到敌方棋子
            }
        }
    }
    return hasAdjacentFriendly;
}
bool Board::isHiveContinuous() const {
    // 如果棋盘为空或只有一个棋子，认为是连续的
    if (grid.empty()) return true;

    // 收集所有已占用的位置
    std::unordered_set<HexCoord> occupied;
    HexCoord startPos;
    bool foundStart = false;

    for (const auto& [coord, hex] : grid) {
        if (!hex.pieces.empty()) {
            occupied.insert(coord);
            if (!foundStart) {
                startPos = coord;
                foundStart = true;
            }
        }
    }

    if (occupied.empty()) return true;

    // 使用DFS探索所有连接的棋子
    std::unordered_set<HexCoord> visited;
    dfsExplore(startPos, visited, occupied);

    // 如果访问过的位置数量等于占用位置的数量，说明蜂巢是连续的
    return visited.size() == occupied.size();
}
bool Board::willMoveMaintainContinuity(const HexCoord &from, const HexCoord &to) const {
    // 临时移除棋子
    auto tempGrid = grid;  // 创建网格的副本

    // 如果起始位置有多个棋子，只移除顶部的棋子
    if (tempGrid.count(from) && !tempGrid[from].pieces.empty()) {
        auto piece = tempGrid[from].pieces.back();
        tempGrid[from].pieces.pop_back();

        // 如果目标位置不存在，创建新的格子
        if (tempGrid.count(to) == 0) {
            tempGrid[to] = Hexagon();
        }
        tempGrid[to].pieces.push_back(piece);

        // 检查移动后的连续性
        // 创建一个临时的board来检查连续性
        Board tempBoard = *this;
        tempBoard.grid = tempGrid;
        return tempBoard.isHiveContinuous();
    }
    return false;
}
bool Board::isEye(const HexCoord& coord) const {
    // 如果是第一个棋子，直接返回false
    if (!firstPiecePlaced) {
        return false;
    }
    // 获取周围的邻居位置
    auto neighbors = coord.neighbors();
    int occupiedCount = 0;
    bool hasEmptySpace = false;
    // 统计被占据的相邻格子
    for (const auto& neighbor : neighbors) {
        if (isValidPosition(neighbor)) {  // 确保邻居在棋盘范围内
            if (isPositionOccupied(neighbor)) {
                occupiedCount++;
            } else {
                hasEmptySpace = true;
            }
        }
    }
    // 如果周围有5个或更多棋子，且至少有一个空位，这个位置就是一个"眼"
    return occupiedCount >= 5 && hasEmptySpace;
}
void Board::afficheneighber(const PlayerID&player) const {
    auto it = queenBeePositions.find(player);
    const HexCoord& queenBeePos = it->second;
    std::vector<HexCoord> neighbors = queenBeePos.neighbors();
    // 检查蜂后周围的六个位置是否全部被占据
    for (const auto& neighbor : neighbors) {
        if (!isPositionOccupied(neighbor)) {
            std::cout<<"The position["<<neighbor.q<<","<<neighbor.r<<"]"<<"is vide"<<std::endl;
        }
    }
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
    std::vector<std::shared_ptr<Piece>> allPieces;
    for (const auto& hex : grid) {
        // 获取该六边形上的所有棋子
        const auto& pieces = hex.second.pieces;
        // 将该六边形上的所有棋子添加到 allPieces 向量中
        allPieces.insert(allPieces.end(), pieces.begin(), pieces.end());
    }
    return allPieces;
}
std::shared_ptr<Piece> Board::getTopPiece(HexCoord coord)const {
    auto hexagonIt = grid.find(coord);
    if (hexagonIt != grid.end()) {
        std::shared_ptr<Piece> topPiece = hexagonIt->second.getTopPiece();
        // 现在可以使用topPiece进行进一步操作
        return  topPiece;
    }
    return nullptr;
}
std::vector<HexCoord> Board::getOccupiedNeighbors(const HexCoord &coord) const {
    std::vector<HexCoord> occupiedNeighbors;
    for (const auto& neighbor : coord.neighbors()) {
        if (isPositionOccupied(neighbor)) {
            occupiedNeighbors.push_back(neighbor);
        }
    }
    return occupiedNeighbors;
}


void Board::printBoard() const {
    std::cout << "\nCurrent board state (Size: " << size << "x" << size << ")" << std::endl;

    // 打印列坐标（顶部）
    printColumnNumbers();

    // 打印棋盘内容
    for (int r = -size; r <= size; ++r) {
        // 打印行号
        std::cout << std::setw(3) << r << " ";

        // 计算缩进
        int indent = std::abs(r);
        std::cout << std::string(indent * 2, ' ');

        // 打印每一行的内容
        for (int q = -size - std::min(0, r); q <= size - std::max(0, r); ++q) {
            HexCoord coord(q, r);
            if (isValidHexPosition(q, r)) {
                printCell(coord);
            } else {
                std::cout << "   ";
            }
        }

        // 打印行尾的行号
        std::cout << " " << std::setw(3) << r << std::endl;
    }

    // 打印列坐标（底部）
    printColumnNumbers();

    // 打印图例
    printLegend();
}

// 添加调试方法
void Board::debugPrintNeighbors(const HexCoord& coord) const {
    std::cout << "Debugging position (" << coord.q << "," << coord.r << "):" << std::endl;
    auto neighbors = coord.neighbors();
    std::cout << "Neighbor positions and occupancy:" << std::endl;
    for (const auto& neighbor : neighbors) {
        std::cout << "(" << neighbor.q << "," << neighbor.r << "): "
                  << (isPositionOccupied(neighbor) ? "Occupied" : "Empty") << std::endl;
    }
}