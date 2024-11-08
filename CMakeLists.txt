cmake_minimum_required(VERSION 3.29)
project(Git_Hive)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 设置源文件编码
if(MSVC)
    add_compile_options("/utf-8")
else()
    add_compile_options(-finput-charset=UTF-8)
endif()
add_executable(Git_Hive main.cpp
        Hive.h
        Board.cpp
        Piece.cpp
        HexCoordHash.h
        HexCoord.h
        Game.cpp
        Game.h
        GameConstants.h
        Worm.cpp
        Aiplayer.cpp
)

# 复制资源到生成的exe文件夹
file(COPY images DESTINATION ${CMAKE_BINARY_DIR})
# 复制资源文件到构建目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
)
