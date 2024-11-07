//
// Created by 李世佳 on 24-11-8.
//

#include "Hive_GUI.hpp"
// 定义初始位置map(放在类定义外)
namespace piecetype {
    const std::map<piecetype::PieceName, sf::Vector2f> piecetype::PieceComponent::initialPiecePositions = {
        {PieceName::Queen, sf::Vector2f(50, 50)},         // 蜂后
        {PieceName::Ant, sf::Vector2f(120, 50)},          // 蚂蚁
        {PieceName::Spider, sf::Vector2f(190, 50)},       // 蜘蛛
        {PieceName::Beetle, sf::Vector2f(260, 50)},       // 甲虫
        {PieceName::Grasshopper, sf::Vector2f(330, 50)},  // 蚱蜢
        {PieceName::Worm, sf::Vector2f(400, 50)}          // 蠕虫
    };
}