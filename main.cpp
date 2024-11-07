#include "Game.h"
#include "Scene_manager.hpp"
int main() {
    // 获取场景管理器的实例
    SceneManager& manager = SceneManager::getInstance();

    // 初始化场景管理器
    manager.init();

    // 运行游戏循环
    manager.run();
    return 0;
}