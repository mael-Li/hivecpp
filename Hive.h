//
// Version 0.03 Created by 李世佳 on 24-10-16.
//READ:完成game的基本搭建，开始了一次游戏
//BUG:无限循环（）
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
public:
    virtual ~Piece() {}
    virtual void move(Board& board, const HexCoord& newPosition) = 0;
    virtual bool isValidMove(const HexCoord& newPosition, const Board& board) const = 0;
    virtual std::string getName() const = 0;
    virtual HexCoord getPosition() const { return position; }
    Piece(const PieceName& name) : name(name) {}
    // 设置棋子的位置
    void setPosition(const HexCoord& pos) {
        position = pos;
    }
    std::shared_ptr<Piece> shared_from_this() {
        return std::enable_shared_from_this<Piece>::shared_from_this();
    }
};
//-------------------------棋盘---------------------------
class Board {
    private:
        int size;
        std::unordered_map<HexCoord, std::shared_ptr<Piece>> grid;
        void initializeGrid();
    public:
        Board(int size):size(size){}
        ~Board(){grid.clear();}
        int getSize()const{return size;}
        void addPiece(std::shared_ptr<Piece> piece, HexCoord coord);
        void removePiece(HexCoord coord);
        std::shared_ptr<Piece> getPieceAt(HexCoord coord) const;
        void printBoard() const;
        bool isValidPosition(HexCoord coord) const;
        bool isPositionOccupied(HexCoord coord) const;
        std::vector<std::shared_ptr<Piece>>getAllPiecesOnBoard(int size)const;
    };

//-------------------------蜂后---------------------------
class QueenBee:public Piece{
    public:
        QueenBee():Piece(PieceName::Queen){}
        //这里重点是实现，蜂后必须在4次操作中被放下，这里的规则或许要写到game里？
        std::string getName() const override{return "Q";}
        bool isValidMove(const HexCoord &newPosition, const Board &board) const override;
        void move(Board &board, const HexCoord& newPosition) override;


};
//-------------------------蚂蚁---------------------------
class Ant:public Piece {
    public://注意设置！！
        Ant():Piece(PieceName::Ant){}
        std::string getName() const override{return "A";}
        bool isValidMove(const HexCoord &newPosition, const Board &board) const override;
        void move(Board &board, const HexCoord& newPosition) override;
        HexCoord getPosition() const override;
    };
} // namespace piecetype

#endif //HIVE_H
