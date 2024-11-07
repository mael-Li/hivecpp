//
// Created by 李世佳 on 24-11-2.
//

#ifndef GAMECONSTANTS_H
#define GAMECONSTANTS_H
#include <map>
#include <string>
namespace piecetype {
    enum class PieceName;
    enum class GameState {
        MENU,
        MAIN,           // 添加 MAIN
        SETTINGS,
        PLAYING,
        GAME_OVER
    };

    // 菜单选项枚举
    enum class MenuOption {
        NONE = -1,
        PLAY_VS_PLAYER,
        PLAY_VS_AI,
        SETTINGS,
        HELP,
        EXIT
    };

    // 动画类型枚举
    enum class AnimationType {
        NONE,
        MOVE,
        PLACE,
        FADE
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
    // 菜单状态枚举
    enum class MenuState {
        MAIN,
        SETTINGS,
        HELP,
        TRANSITIONING
    };

    // 声明全局常量
    extern const std::map<std::string, PieceName> PIECE_TYPES;
    extern const std::map<GameCommand, std::string> COMMAND_DESCRIPTIONS;
}
#endif //GAMECONSTANTS_H
