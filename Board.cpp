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
    // 1. 空指针检查
    if (!piece) {
        throw std::invalid_argument("Cannot add null piece");
    }

    // 2. 验证玩家ID
    if (piece->getID() != player) {
        throw std::invalid_argument("Piece does not belong to the current player");
    }

    // 3. 检查坐标是否有效
    if (!isValidPosition(coord)) {
        throw std::invalid_argument("Invalid coordinate position");
    }

    // 4. 创建或获取格子
    if (grid.find(coord) == grid.end()) {
        grid[coord] = Hexagon();
    }

    // 5. 更新可用棋子数量
    auto pieceType = piece->getEumName();
    if (piecesAvailable[player][pieceType] <= 0) {
        throw std::runtime_error("No more pieces of this type available");
    }
    piecesAvailable[player][pieceType]--;

    // 6. 添加棋子到格子,并且更新棋子位置
    piece->setPosition(coord);
    grid[coord].pieces.push_back(piece);

    // 7. 更新首次放置状态
    if (!firstPiecePlaced) {
        firstPiecePlaced = true;
    }
    // 8. 如果是蜂后，更新蜂后位置
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
bool Board::canPlacePiece(const HexCoord &coord, PlayerID playerid) const {
    // 第一个棋子可以放在任何位置
    if (!firstPiecePlaced) return true;

    // 验证是否与自己的棋子相邻
    bool adjacentToFriendly = false;
    bool adjacentToEnemy = false;

    for (const auto& neighbor : coord.neighbors()) {
        if (isPositionOccupied(neighbor)) {
            auto piece = getTopPiece(neighbor);
            if (piece) {
                if (piece->getID() == playerid)
                    adjacentToFriendly = true;
                else
                    adjacentToEnemy = true;
            }
        }
    }

    // 必须与自己的棋子相邻，但不能与敌方棋子相邻
    return adjacentToFriendly && !adjacentToEnemy;
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
    index++;
    int a = index;
    std::cout << "Board[" << a << "]" << std::endl;

    for (int row = size; row >= -size; --row) {
        for (int col = -size; col < size; ++col) {
            HexCoord coord(col, row);
            if (isValidPosition(coord)) {
                std::shared_ptr<Piece> piece = getTopPiece(coord);
                if (piece) {
                    // 打印当前六边形的所有棋子
                    std::cout << "[";
                    for (const auto& p : grid.at(coord).pieces) {
                        std::string player = (p->getID() == PlayerID::player1) ? "1" : "2";
                        std::cout << p->getName() << player << " ";
                    }
                    std::cout << "]";
                } else {
                    std::cout << "[   ]";
                }
            }
        }
        std::cout << std::endl;
    }
}
void Board::printPieceInfoAt(const HexCoord& coord) const {
    auto it = grid.find(coord);
    if (it != grid.end() && !it->second.pieces.empty()) {
        // 如果找到了棋子，打印棋子的类型和坐标
        const auto& piece = it->second.pieces.back(); // 假设我们打印最上面的棋子
        std::cout << "Piece at (" << coord.q << ", " << coord.r << "): " << piece->getName() << std::endl;
    } else {
        // 如果没有找到棋子，打印空位置信息
        std::cout << "No piece at (" << coord.q << ", " << coord.r << ")" << std::endl;
    }
}
