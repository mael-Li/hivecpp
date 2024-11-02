#include "Game.h"
#include "Scene_manager.hpp"
int main() {
    try {
        auto& sceneManager = SceneManager::getInstance();
        sceneManager.init();
        sceneManager.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}