#include "Game.h"
int main() {
    using namespace piecetype;
    Game::displayStartScreen();
    bool isPvP = Game::getGameMode();
    bool useExtendedPieces = Game::getUseExtendedPieces();
    // 创建游戏实例并开始游戏
    Game game(isPvP, useExtendedPieces);
    game.start();
    return 0;
}