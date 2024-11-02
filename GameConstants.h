//
// Created by 李世佳 on 24-11-2.
//

#ifndef GAMECONSTANTS_H
#define GAMECONSTANTS_H
#include <map>
#include <string>

namespace piecetype {
        // 游戏状态枚举
        enum class GameState {
            MENU,
            PLAYING,
            PAUSED,
            GAME_OVER
        };

        // 游戏命令枚举
        enum class GameCommand {
            PLACE_PIECE,
            MOVE_PIECE,
            SHOW_HELP,
            SHOW_BOARD,
            SHOW_STATS,
            QUIT,
            INVALID
        };

        // 声明全局常量
        extern const std::map<std::string, PieceName> PIECE_TYPES;
        extern const std::map<GameCommand, std::string> COMMAND_DESCRIPTIONS;
}
#endif //GAMECONSTANTS_H
