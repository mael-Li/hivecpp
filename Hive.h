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
static int index=0;
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
    std::shared_ptr<Piece> piece;
    // 默认构造函数，默认构造函数储存一个指针
    Hexagon() : piece(nullptr) {}

    Hexagon(HexCoord coord) : coord(coord), piece(nullptr) {}

    void setPiece(std::shared_ptr<Piece> piece) {
        this->piece = piece;
    }
};
// 定义棋子基类
class Piece : public std::enable_shared_from_this<Piece>{
protected:
    HexCoord position;
    PieceName name;
    PlayerID ID;
public:
    virtual ~Piece() {}
    virtual void move(Board& board, const HexCoord& newPosition) = 0;
    virtual bool isValidMove(const HexCoord& newPosition, const Board& board) const = 0;
    virtual std::string getName() const = 0;
    virtual HexCoord getPosition() const { return position; }
    Piece(const PieceName& name,const PlayerID &player) : name(name),ID(player){}
    //这里是返回对应类型的名字
    PieceName getEumName()const {return name;}
    // 设置棋子的位置
    void setPosition(const HexCoord& pos) {
        position = pos;
    }
    PlayerID getID()const{return ID;}
    void setID(const PlayerID& a){ID = a;}
    //重载这个标准库的函数让其可以正常读取这是个指针的目前值
    std::shared_ptr<Piece> shared_from_this() {
        return std::enable_shared_from_this<Piece>::shared_from_this();
    }
};
//-------------------------棋盘---------------------------
class Board {
    private:
        int size;
        std::unordered_map<HexCoord, std::shared_ptr<Piece>> grid;
        std::unordered_map<PlayerID, std::unordered_map<PieceName, int>> piecesAvailable;
        std::unordered_map<PlayerID, HexCoord>  queenBeePositions;
        void initializeGrid();
        //初始化棋子数量
        void initializePiecesAvailable();
    public:
        //构造器，棋盘规模
        Board(int size):size(size){initializePiecesAvailable();}
        ~Board(){grid.clear();}
        int getSize()const{return size;}
    //棋盘的基本行为，添加棋子和删除棋子
        void addPiece(std::shared_ptr<Piece> piece, HexCoord coord,PlayerID);
        void removePiece(HexCoord coord);
    //这里用于在添加棋子时，如果输入的是queen棋子可以直接保存其位置queenBeePositions;
        void setqueenBeePositions(const HexCoord& c,PlayerID a){queenBeePositions.emplace(a,c);}
        //获得这个棋子，并且是智能指针类型
        std::shared_ptr<Piece> getPieceAt(HexCoord coord) const;
        void printBoard() const;
    //检查位置合法性
        bool isValidPosition(HexCoord coord) const;
        bool isPositionOccupied(HexCoord coord) const;
        bool ishasNeighber(HexCoord coord)const;
        bool isQueenBeeSurround(PlayerID)const;
    //打印蜂后邻居的控制位置，用于调试代码
        void afficheneighber(const PlayerID&)const;
        PlayerID checkVictory()const;
    //获得目前棋盘上的所有棋子
        std::vector<std::shared_ptr<Piece>>getAllPiecesOnBoard(int size)const;
    };

//-------------------------蜂后---------------------------
class QueenBee:public Piece{
    public:
        QueenBee(PlayerID player):Piece(PieceName::Queen,player){}
        //这里重点是实现，蜂后必须在4次操作中被放下，这里的规则或许要写到game里？
        std::string getName() const override{return "Q";}
        bool isValidMove(const HexCoord &newPosition, const Board &board) const override;
        void move(Board &board, const HexCoord& newPosition) override;


};
//-------------------------蚂蚁---------------------------
class Ant:public Piece {
    public://注意设置！！
        Ant(PlayerID player):Piece(PieceName::Ant,player){}
        std::string getName() const override{return "A";}
        bool isValidMove(const HexCoord &newPosition, const Board &board) const override;
        void move(Board &board, const HexCoord& newPosition) override;
        HexCoord getPosition() const override;
    };
} // namespace piecetype

#endif //HIVE_H
