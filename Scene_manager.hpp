//
// Created by 李世佳 on 24-11-2.
//

#ifndef SCENE_MANAGER_HPP
#define SCENE_MANAGER_HPP
#include <SFML/Graphics.hpp>
#include <memory>
#include "Hive.h"
#include "GameConstants.h"  // 包含枚举和常量定义
#include "Game.h"          // 包含Game类定义
using namespace piecetype;
// 前向声明
const std::string FONT_PATH = "C:\\Windows\\Fonts\\consola.ttf";
const std::string RESOURCE_PATH = "resources/";
class Scene;
const std::string GAME_MESSAGES[] = {
    "Your turn",
    "Waiting for opponent",
    "Game Over",
    "Victory!",
    "Defeat",
    "Draw",
    "Invalid Move",
    "Place your Queen",
    "Move your piece"
};

class SceneManager {
public:
    static SceneManager& getInstance() {
        static SceneManager instance;
        return instance;
    }

    void init();
    void run();
    void setScene(std::unique_ptr<Scene> scene);
    sf::RenderWindow& getWindow() { return window; }

private:
    SceneManager() = default;
    sf::RenderWindow window;
    std::unique_ptr<Scene> currentScene;
};

class Scene {
public:
    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update() = 0;
    virtual void render(sf::RenderWindow& window) = 0;
    virtual ~Scene() = default;
};
class Button {
public:
    static const std::string FONT_PATH;

    bool loadFont();

    Button(const std::string& text, const sf::Vector2f& position, const sf::Vector2f& size);
    bool contains(sf::Vector2f point) const;
    void setHovered(bool hovered);
    void render(sf::RenderWindow& window);

private:
    sf::RectangleShape rect;
    sf::Text buttonText;
    sf::Font font;
};
enum class GameMode {
    PLAYER_VS_PLAYER,
    PLAYER_VS_AI
};

class MainMenuScene : public Scene {
public:
    MainMenuScene();
    void handleEvent(const sf::Event& event) override;
    void update() override;
    void render(sf::RenderWindow& window) override;

private:
    sf::Font font;
    std::vector<std::unique_ptr<Button>> buttons;
};

class GameModeScene : public Scene {
public:
    GameModeScene();
    void handleEvent(const sf::Event& event) override;
    void update() override;
    void render(sf::RenderWindow& window) override;

private:
    std::vector<std::unique_ptr<Button>> buttons;
    sf::Font font;
    void createButtons();  // 新增函数用于创建按钮
};
class GameplayScene : public Scene {
public:
    explicit GameplayScene(GameMode mode);
    void handleEvent(const sf::Event& event) override;
    void update() override;
    void render(sf::RenderWindow& window) override;

private:
    GameMode gameMode;
    std::unique_ptr<piecetype::Game> game;

    // 游戏棋盘绘制相关
    static constexpr float HEX_SIZE = 40.0f;
    static constexpr float BOARD_OFFSET_X = 400.0f;
    static constexpr float BOARD_OFFSET_Y = 300.0f;

    // 棋子纹理和精灵
    std::map<piecetype::PieceName, sf::Texture> pieceTextures;
    std::map<piecetype::PlayerID, sf::Color> playerColors;

    // UI元素
    sf::Font font;
    sf::Text statusText;
    sf::RectangleShape selectedPieceHighlight;

    // 状态变量
    HexCoord selectedPiecePos;
    bool isPieceSelected;

    // 辅助函数
    void initializeGraphics();
    void loadTextures();
    sf::Vector2f hexToScreen(const HexCoord& hex);
    HexCoord screenToHex(const sf::Vector2f& screen);
    void drawHexagon(sf::RenderWindow& window, const HexCoord& pos, sf::Color color);
    void drawPiece(sf::RenderWindow& window, const piecetype::Piece& piece);
    void drawBoard(sf::RenderWindow& window);
    void handleMouseClick(sf::Vector2i mousePos);
    void updateStatusText();
};

#endif //SCENE_MANAGER_HPP
