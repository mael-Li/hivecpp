cmake_minimum_required(VERSION 3.29)
project(Git_Hive)

set(CMAKE_CXX_STANDARD 14)

add_executable(Git_Hive main.cpp
        Hive.h
        Board.cpp
        QueenBee.cpp
        Piece.cpp
        HexCoordHash.h
        HexCoord.h
)
