cmake_minimum_required(VERSION 3.29)
project(Git_Hive)

set(CMAKE_CXX_STANDARD 14)

set(SDL2_DIR E:/BaiduNetdiskDownload/SDL2-2.26.5/x86_64-w64-mingw32)
include_directories(E:/BaiduNetdiskDownload/SDL2-2.26.5/x86_64-w64-mingw32/include)
link_directories(E:/BaiduNetdiskDownload/SDL2-2.26.5/x86_64-w64-mingw32/bin)
add_executable(Git_Hive main.cpp
        Hive.h
        Board.cpp
        Piece.cpp
        HexCoordHash.h
        HexCoord.h
        Game.cpp
        Game.h
)
target_link_libraries(Git_Hive mingw32 SDL2 SDL2main)