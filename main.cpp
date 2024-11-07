#include "Game.h"
#include "Hive_GUI.hpp"

int main() {
    try {
        piecetype::HiveGUI game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
