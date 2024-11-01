//
// Version 0.06 Created by 李世佳 on 24-10-20.
//READ:目前加入了不同玩家所属的棋子
//BUG: 没有加入基于棋子的玩家id来进行移动的项目
//
#pragma once
#ifndef HIVE_H
#define HIVE_H
#include "HexCoord.h"
#include "HexCoordHash.h"
#include <random>
#include <vector>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <iomanip>
#include <windows.h>
static int index;
static int index_ai;
namespace piecetype {
    class Board;
    class Piece;
    enum class PieceName { Queen, Ant, Spider, Beetle, Grasshopper };
    enum class PlayerID {player1,player2,playerai,playernobody};
// 定义蜂巢坐标结构

// 定义蜂巢格子类
class Hexagon {
public:
    HexCoord coord;
    std::vector<std::shared_ptr<Piece>> pieces;
    //std::shared_ptr<Piece> piece;
    // 默认构造函数，默认构造函数储存一个指针
    Hexagon() : pieces(){}//piece(nullptr),

    Hexagon(HexCoord coord) : coord(coord),pieces() {}//piece(nullptr)
    // 移除最上方的棋子
    std::shared_ptr<Piece> removeTopPiece() {
        auto topPiece = pieces.back();
        pieces.pop_back();
        return topPiece;
    }
    // 获取最上方的棋子
    std::shared_ptr<Piece> getTopPiece() const {
        if (!pieces.empty()) {
            return pieces.back();
        }
        return nullptr;
    }

    void addPiece(std::shared_ptr<Piece> piece) {
        pieces.push_back(piece);
    }
};
// 定义棋子基类
class Piece : public std::enable_shared_from_this<Piece>{
protected:
    HexCoord position;
    PieceName name;
    PlayerID ID;
    std::vector<HexCoord> moveHistory;

public:
    virtual ~Piece() {}
    virtual void move(Board& board, const HexCoord& newPosition,const PlayerID&) = 0;
    virtual bool isValidMove(const HexCoord& newPosition, const Board& board) const = 0;
    virtual std::string getName() const = 0;
    virtual HexCoord getPosition() const = 0;
    Piece(const PieceName& name,const PlayerID &player) : name(name),ID(player){}
    //这里是返回对应类型的名字
    PieceName getEumName()const {return name;}
    // 设置棋子的位置
    void setPosition(const HexCoord& pos) {
        position = pos;
    }
    PlayerID getID()const{return ID;}
    void setID(const PlayerID& a){ID = a;}
    //修改
    /*
    virtual bool canMove(const Board& board) const {
        // 检查是否被完全包围
        HexCoord pos = getPosition();
        auto neighbors = pos.neighbors();
        bool surrounded = true;
        for (const auto& neighbor : neighbors) {
            if (!board.isPositionOccupied(neighbor)) {
                surrounded = false;
                break;
            }
        }
        return !surrounded;
    }
    */
    //修改
    void recordMove(const HexCoord& newPos) {
        moveHistory.push_back(newPos);
    }
    //修改
    HexCoord getLastPosition() const {
        return moveHistory.empty() ? position : moveHistory.back();
    }
    //重载这个标准库的函数让其可以正常读取这是个指针的目前值
    std::shared_ptr<Piece> shared_from_this() {
        return std::enable_shared_from_this<Piece>::shared_from_this();
    }
};
//-------------------------棋盘异常-----------------------
class Pieceexception :public std::exception{
    std::string info;
public:
    Pieceexception(const char* s)throw():info(s){}
    const char* what() const noexcept {return info.c_str();}
};
//-------------------------棋盘---------------------------
class Board {
    private:
        int size;
        std::unordered_map<HexCoord, Hexagon> grid;
        std::unordered_map<PlayerID, HexCoord> queenBeePositions;
        bool firstPiecePlaced = false;
        // 添加以下成员变量来跟踪游戏状态
        bool queenPlacementRequired[2] = {false, false}; // 跟踪是否需要放置蜂后
        int turnCount = 0; // 回合计数
void dfsExplore(const HexCoord& current,
            std::unordered_set<HexCoord>& visited,
            const std::unordered_set<HexCoord>& occupied) const {
        visited.insert(current);
        // 检查所有相邻位置
        for (const auto& neighbor : current.neighbors()) {
            // 如果邻居位置有棋子且未访问过
            if (occupied.count(neighbor) && !visited.count(neighbor)) {
                dfsExplore(neighbor, visited, occupied);
            }
        }
    }
        void initializeGrid();
        //初始化棋子数量

    public:
        //构造器，棋盘规模
         std::unordered_map<PlayerID, std::unordered_map<PieceName, int>> piecesAvailable;
        Board(int size):size(size){initializePiecesAvailable();}
        ~Board(){grid.clear();}
        int getSize()const{return size;}
        void initializePiecesAvailable();
    //棋盘的基本行为，添加棋子和删除棋子
        void addPiece(std::shared_ptr<Piece> piece, HexCoord coord,PlayerID);
        std::shared_ptr<Piece> removePiece(HexCoord coord);
    //这里用于在添加棋子时，如果输入的是queen棋子可以直接保存其位置queenBeePositions;
        void setqueenBeePositions(const HexCoord& c,PlayerID a){queenBeePositions.emplace(a,c);}
        //获得这个棋子，并且是智能指针类型
        void printBoard() const;
    //检查位置合法性
        bool isValidPosition(HexCoord coord) const;
        bool isPositionOccupied(HexCoord coord) const;
        bool ishasNeighber(HexCoord coord)const;
        bool isQueenBeeSurround(PlayerID)const;
        bool isTopPiece(const HexCoord& coord, PlayerID id)const;
        bool canPlacePiece(const HexCoord& coord,PlayerID playerid)const;
    //！！对于棋盘的连续性检测
        bool isHiveContinuous() const;
        bool isValidMove(const HexCoord& from, const HexCoord& to) const;
    //！！检测移动后是否会对连续性有影响
        bool willMoveMaintainContinuity(const HexCoord& from, const HexCoord& to) const;
    //检查是不是眼
        bool isEye(const HexCoord& coord)const;
    //打印蜂后邻居的控制位置，用于调试代码
        void afficheneighber(const PlayerID&)const;
        void debugPrintNeighbors(const HexCoord& coord) const;
        PlayerID checkVictory()const;
    //获得目前棋盘上的所有棋子
        std::vector<std::shared_ptr<Piece>>getAllPiecesOnBoard(int size)const;
    //获得目标位置的棋子
        std::shared_ptr<Piece> getPieceAt(HexCoord coord) const;
    //获得TOP棋子
        std::shared_ptr<Piece> getTopPiece(HexCoord coord)const;
    //获取所有邻居位置
    std::vector<HexCoord> getOccupiedNeighbors(const HexCoord& coord) const;
    //回合管理的方法
    void nextTurn() {
        turnCount++;
        // 第四回合后必须放置蜂后
        if (turnCount == 4) {
            queenPlacementRequired[0] = true;
            queenPlacementRequired[1] = true;
        }
    }
    void printNeighbors(const HexCoord& coord) const {
        std::vector<HexCoord> neighbors = coord.neighbors();
        std::cout << "Neighbors of (" << coord.q << ", " << coord.r << "):" << std::endl;

        for (const auto& neighbor : neighbors) {
            std::cout << "(" << neighbor.q << ", " << neighbor.r << ")" << std::endl;
        }
    }
    int getTotalPieces() const {
        int count = 0;
        for (const auto& [coord, hexagon] : grid) {
            count += hexagon.pieces.size();
        }
        return count;
    }

    private:
    void clearQueenBeePosition(PlayerID playerId) {
        queenBeePositions.erase(playerId);
    }
    bool isValidHexPosition(int q, int r) const {
        int s = -q - r;  // 计算第三个轴的坐标
        return std::abs(q) <= size &&
               std::abs(r) <= size &&
               std::abs(s) <= size;
    }
    void printCell(const HexCoord& coord) const {
        auto piece = getTopPiece(coord);
        if (piece) {
            // 打印棋子信息
            std::string pieceStr = piece->getName();
            PlayerID playerId = piece->getID();
            std::cout << "[" << pieceStr
                     << (playerId == PlayerID::player1 ? "1" : "2")
                     << "]";
        } else {
            // 打印空格子
            std::cout << "[  ]";
        }
    }
    // 打印列号
    void printColumnNumbers() const {
        std::cout << "   ";  // 初始缩进
        for (int q = -size; q <= size; ++q) {
            std::cout << std::setw(4) << q;
        }
        std::cout << std::endl;
    }
    //添加图例
    void printLegend() const {
        std::cout << "\nLegend:" << std::endl;

        // 玩家1的棋子
        std::cout << "Player 1 Pieces:" << std::endl;
        std::cout << "  [Q1] - Queen       (Remaining: "
                  << piecesAvailable.at(PlayerID::player1).at(PieceName::Queen) << ")" << std::endl;
        std::cout << "  [A1] - Ant         (Remaining: "
                  << piecesAvailable.at(PlayerID::player1).at(PieceName::Ant) << ")" << std::endl;
        std::cout << "  [B1] - Beetle      (Remaining: "
                  << piecesAvailable.at(PlayerID::player1).at(PieceName::Beetle) << ")" << std::endl;
        std::cout << "  [S1] - Spider      (Remaining: "
                  << piecesAvailable.at(PlayerID::player1).at(PieceName::Spider) << ")" << std::endl;
        std::cout << "  [G1] - Grasshopper (Remaining: "
                  << piecesAvailable.at(PlayerID::player1).at(PieceName::Grasshopper) << ")" << std::endl;

        std::cout << "\nPlayer 2 Pieces:" << std::endl;
        std::cout << "  [Q2] - Queen       (Remaining: "
                  << piecesAvailable.at(PlayerID::player2).at(PieceName::Queen) << ")" << std::endl;
        std::cout << "  [A2] - Ant         (Remaining: "
                  << piecesAvailable.at(PlayerID::player2).at(PieceName::Ant) << ")" << std::endl;
        std::cout << "  [B2] - Beetle      (Remaining: "
                  << piecesAvailable.at(PlayerID::player2).at(PieceName::Beetle) << ")" << std::endl;
        std::cout << "  [S2] - Spider      (Remaining: "
                  << piecesAvailable.at(PlayerID::player2).at(PieceName::Spider) << ")" << std::endl;
        std::cout << "  [G2] - Grasshopper (Remaining: "
                  << piecesAvailable.at(PlayerID::player2).at(PieceName::Grasshopper) << ")" << std::endl;

        // 空格子说明
        std::cout << "\n  [ ] - Empty cell" << std::endl;
    }
    void printDebugInfo() const {
        std::cout << "\nDebug Information:" << std::endl;
        std::cout << "Board size: " << size << std::endl;
        std::cout << "Total pieces: " << grid.size() << std::endl;
        // 添加其他可能需要的调试信息
    }
};

//-------------------------蜂后---------------------------
class QueenBee:public Piece{
    public:
        QueenBee(PlayerID player):Piece(PieceName::Queen,player){}
        //这里重点是实现，蜂后必须在4次操作中被放下，这里的规则或许要写到game里？
        std::string getName() const override{return "Q";}
        bool isValidMove(const HexCoord &newPosition, const Board &board) const override;
        void move(Board &board, const HexCoord& newPosition,const PlayerID&) override;
        bool canSlideTo(const HexCoord& newposition,const Board& board)const;
        HexCoord getPosition() const override;
};
//-------------------------蚂蚁---------------------------
class Ant:public Piece {
    public://注意设置！！
        Ant(PlayerID player):Piece(PieceName::Ant,player){}
        std::string getName() const override{return "A";}
        bool isValidMove(const HexCoord &newPosition, const Board &board) const override;
        void move(Board &board, const HexCoord& newPosition,const PlayerID&) override;
        HexCoord getPosition() const override;
};

class Spider : public Piece {
public:
    Spider(PlayerID player) : Piece(PieceName::Spider, player) {}
    std::string getName() const override { return "S"; }
    bool isValidMove(const HexCoord &newPosition, const Board &board) const override;
    void move(Board &board, const HexCoord &newPosition, const PlayerID &) override;
    HexCoord getPosition() const override;
};

class Grasshopper : public Piece {
public:
    Grasshopper(PlayerID player) : Piece(PieceName::Grasshopper, player) {}
    std::string getName() const override { return "G"; }
    bool isValidMove(const HexCoord &newPosition, const Board &board) const override;
    void move(Board &board, const HexCoord &newPosition, const PlayerID &) override;
    HexCoord getPosition() const override;
};
class Beetle : public Piece {
public:
    Beetle(PlayerID player) : Piece(PieceName::Beetle, player) {}
    std::string getName() const override { return "B"; }
    bool isValidMove(const HexCoord &newPosition, const Board &board) const override;
    void move(Board &board, const HexCoord &newPosition, const PlayerID &) override;
    HexCoord getPosition() const override;
};
    //EXCEPTION--------------------------------------------------------------------
    class QueenNotPlacedException : public Pieceexception {
    public:
        QueenNotPlacedException() : Pieceexception("Queen must be placed by turn 4") {}
    };

    class InvalidMoveException : public Pieceexception {
    public:
        InvalidMoveException(const std::string& reason)
            : Pieceexception(("Invalid move: " + reason).c_str()) {}
    };

    class HiveContinuityException : public std::runtime_error {
    public:
        explicit HiveContinuityException(const std::string& message)
            : std::runtime_error(message) {}
    };
} // namespace piecetype

#endif //HIVE_H