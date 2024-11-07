#ifndef SCENE_MANAGER_HPP
#define SCENE_MANAGER_HPP

#include <SFML/Graphics.hpp>
#include <memory>
#include <stack>
#include <stdexcept>
#include "Game.h"
#include <filesystem>
#include <vector>
enum class SceneType {
    START,
    MODE,
    GAME
};

class FontManager {
private:
    FontManager() = default;
    sf::Font gameFont;
    bool isFontLoaded = false;

    // Windows 系统字体路径
    const std::vector<std::string> systemFontPaths = {
        "C:\\Windows\\Fonts\\arial.ttf",
        "C:\\Windows\\Fonts\\segoe.ttf",
        "C:\\Windows\\Fonts\\calibri.ttf"
    };

    // Linux 系统字体路径
    const std::vector<std::string> linuxSystemFonts = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/TTF/Arial.ttf"
    };

public:
    static FontManager& getInstance() {
        static FontManager instance;
        return instance;
    }

    bool loadGameFont() {
        // 1. 首先尝试加载项目目录中的字体
        std::vector<std::string> fontPaths = {
            "assets/fonts/NotoSans-Black.ttf",
            "./fonts/NotoSans-Black.ttf",
            "../fonts/NotoSans-Black.ttf",
            "./NotoSans-Black.ttf",
            "../NotoSans-Black.ttf"
        };

        // 打印当前工作目录以便调试
        std::cout << "Current working directory: "
                  << std::filesystem::current_path() << std::endl;

        // 尝试加载项目字体
        for (const auto& path : fontPaths) {
            std::cout << "Trying to load font from: " << path << std::endl;
            if (gameFont.loadFromFile(path)) {
                std::cout << "Successfully loaded font from: " << path << std::endl;
                isFontLoaded = true;
                return true;
            }
        }

        // 2. 如果项目字体加载失败，尝试系统字体
        #ifdef _WIN32
            for (const auto& path : systemFontPaths) {
                std::cout << "Trying to load system font from: " << path << std::endl;
                if (gameFont.loadFromFile(path)) {
                    std::cout << "Successfully loaded system font from: " << path << std::endl;
                    isFontLoaded = true;
                    return true;
                }
            }
        #else
            for (const auto& path : linuxSystemFonts) {
                std::cout << "Trying to load system font from: " << path << std::endl;
                if (gameFont.loadFromFile(path)) {
                    std::cout << "Successfully loaded system font from: " << path << std::endl;
                    isFontLoaded = true;
                    return true;
                }
            }
        #endif

        // 3. 如果所有字体都加载失败
        std::cerr << "Failed to load any font!" << std::endl;
        return false;
    }

    const sf::Font& getFont() const {
        if (!isFontLoaded) {
            throw std::runtime_error("No font loaded! Call loadGameFont first.");
        }
        return gameFont;
    }

    bool isLoaded() const {
        return isFontLoaded;
    }
};

class Scene {
public:
    virtual ~Scene() = default;
    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update() = 0;
    virtual void render(sf::RenderWindow& window) = 0;
};

class SceneManager {
private:
    SceneManager() = default;
    sf::RenderWindow window;
    std::stack<std::unique_ptr<Scene>> scenes;

public:
    static SceneManager& getInstance() {
        static SceneManager instance;
        return instance;
    }

    void init() {
        window.create(sf::VideoMode(1024, 768), "Hive Game");
        window.setFramerateLimit(60);

        // 尝试加载字体
        if (!FontManager::getInstance().loadGameFont()) {
            std::cerr << "Warning: Failed to load any font. The game may not display text correctly." << std::endl;
            // 游戏仍然会继续运行，但可能没有文字显示
        }

        pushScene(SceneType::START);
    }

    void run() {
        while (window.isOpen() && !scenes.empty()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
                scenes.top()->handleEvent(event);
            }

            scenes.top()->update();
            
            window.clear(sf::Color::White);
            scenes.top()->render(window);
            window.display();
        }
    }

    void pushScene(SceneType type);
    void popScene() {
        if (!scenes.empty())
            scenes.pop();
    }

    sf::RenderWindow& getWindow() { return window; }
};

// HexagonPiece class for rendering individual pieces
class HexagonPiece {
private:
    static constexpr float HEX_SIZE = 30.0f;
    sf::CircleShape hexagon;
    sf::Text text;
    sf::Vector2f originalPosition;
    sf::Vector2f position;
    bool isDragging = false;
    sf::Vector2f dragOffset;
    bool isPlayer1;
public:
    HexagonPiece(const std::string& letter, const sf::Font& font,
                 const sf::Vector2f& pos, bool isPlayer1)
        : position(pos), originalPosition(pos), isPlayer1(isPlayer1) {

        hexagon.setPointCount(6);
        hexagon.setRadius(HEX_SIZE);
        hexagon.setRotation(30.f);
        hexagon.setPosition(pos);
        hexagon.setFillColor(isPlayer1 ? sf::Color::White : sf::Color(200, 200, 200));
        hexagon.setOutlineThickness(2.f);
        hexagon.setOutlineColor(sf::Color::Black);

        text.setFont(font);
        text.setString(letter);
        text.setCharacterSize(24);
        text.setFillColor(sf::Color::Black);

        // Center the text in the hexagon
        sf::FloatRect textBounds = text.getLocalBounds();
        text.setOrigin(textBounds.width/2, textBounds.height/2);
        text.setPosition(pos.x + HEX_SIZE, pos.y + HEX_SIZE);
    }

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
        if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f mousePos = window.mapPixelToCoords(
                {event.mouseButton.x, event.mouseButton.y});
            if (hexagon.getGlobalBounds().contains(mousePos)) {
                isDragging = true;
                dragOffset = position - mousePos;
            }
        }
        else if (event.type == sf::Event::MouseButtonReleased) {
            isDragging = false;
        }
        else if (event.type == sf::Event::MouseMoved && isDragging) {
            sf::Vector2f mousePos = window.mapPixelToCoords(
                {event.mouseMove.x, event.mouseMove.y});
            position = mousePos + dragOffset;
            updatePosition();
        }
    }

    void snapToPosition(const sf::Vector2f& newPos) {
        position = newPos;
        updatePosition();
        isDragging = false;
    }
    void resetPosition() {
        position = originalPosition;
        updatePosition();
    }
    void updatePosition() {
        hexagon.setPosition(position);
        text.setPosition(position.x + HEX_SIZE, position.y + HEX_SIZE);
    }
    bool isDragging_()const{ return isDragging; }

    void draw(sf::RenderWindow& window) {
        window.draw(hexagon);
        window.draw(text);
    }
};

// StartScene class
class StartScene : public Scene {
private:
    sf::Text title;
    sf::RectangleShape startButton;
    sf::Text buttonText;
    bool fontLoaded;
public:
    StartScene() : fontLoaded(false) {
        // 检查字体是否已加载
        if (FontManager::getInstance().isLoaded()) {
            fontLoaded = true;
            const sf::Font& font = FontManager::getInstance().getFont();

            title.setFont(font);
            title.setString("Hive Game");
            title.setCharacterSize(72);
            title.setFillColor(sf::Color::Black);
            title.setPosition(400, 200);

            buttonText.setFont(font);
            buttonText.setString("Start Game");
            buttonText.setCharacterSize(24);
            buttonText.setFillColor(sf::Color::White);
            buttonText.setPosition(450, 410);
        } else {
            std::cerr << "Warning: Font not loaded in StartScene. Text will not be displayed." << std::endl;
        }

        // 这些不需要字体的组件总是被初始化
        startButton.setSize(sf::Vector2f(200, 50));
        startButton.setPosition(412, 400);
        startButton.setFillColor(sf::Color::Green);
    }

    void handleEvent(const sf::Event& event) override {
        if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(SceneManager::getInstance().getWindow());
            if (startButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                SceneManager::getInstance().pushScene(SceneType::MODE);
            }
        }
    }

    void update() override {}

    void render(sf::RenderWindow& window) override {
        window.draw(startButton);
        if (fontLoaded) {
            window.draw(title);
            window.draw(buttonText);
        }
    }
};

// ModeScene class
class ModeScene : public Scene {
private:
    sf::Font font;
    sf::Text title;
    sf::RectangleShape pvpButton;
    sf::RectangleShape pvcButton;
    sf::Text pvpText;
    sf::Text pvcText;

public:
    ModeScene() {
        try {
            const sf::Font& font = FontManager::getInstance().getFont();
            title.setFont(font);
            title.setString("Select Game Mode");
            title.setCharacterSize(48);
            title.setFillColor(sf::Color::Black);
            title.setPosition(350, 200);

            pvpButton.setSize(sf::Vector2f(200, 50));
            pvpButton.setPosition(412, 350);
            pvpButton.setFillColor(sf::Color::Blue);

            pvcButton.setSize(sf::Vector2f(200, 50));
            pvcButton.setPosition(412, 450);
            pvcButton.setFillColor(sf::Color::Red);

            pvpText.setFont(font);
            pvpText.setString("Player vs Player");
            pvpText.setCharacterSize(24);
            pvpText.setFillColor(sf::Color::White);
            pvpText.setPosition(430, 360);

            pvcText.setFont(font);
            pvcText.setString("Player vs Computer");
            pvcText.setCharacterSize(24);
            pvcText.setFillColor(sf::Color::White);
            pvcText.setPosition(420, 460);
        }
        catch (const std::runtime_error& e) {
            // 处理字体加载错误
            std::cerr << "Error in ModeScene: " << e.what() << std::endl;
            // 可以在这里设置一个默认的系统字体或者显示错误信息
        }
    }


    void handleEvent(const sf::Event& event) override {
        if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(SceneManager::getInstance().getWindow());
            if (pvpButton.getGlobalBounds().contains(mousePos.x, mousePos.y) ||
                pvcButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                SceneManager::getInstance().pushScene(SceneType::GAME);
            }
        }
    }

    void update() override {}

    void render(sf::RenderWindow& window) override {
        window.draw(title);
        window.draw(pvpButton);
        window.draw(pvcButton);
        window.draw(pvpText);
        window.draw(pvcText);
    }
};

// GameScene class
class GameScene : public Scene {
private:
    std::vector<HexagonPiece> player1Pieces;  // 玩家1的棋子
    std::vector<HexagonPiece> player2Pieces;  // 玩家2的棋子
    std::map<std::string, int> player1PieceCounts;  // 玩家1的棋子数量
    std::map<std::string, int> player2PieceCounts;  // 玩家2的棋子数量
    piecetype::Game game;
    bool isPlayer1Turn = true;  // 当前回合玩家
    // 存储棋盘格子的位置
    std::vector<std::pair<sf::Vector2f, bool>> gridPositions;
    static constexpr float HEX_SIZE = 30.0f;
    static constexpr float HORIZONTAL_SPACING = HEX_SIZE * 1.732f;  // √3 * HEX_SIZE
    static constexpr float VERTICAL_SPACING = HEX_SIZE * 1.5f;
public:
    GameScene() {
        if (!FontManager::getInstance().isLoaded()) {
            std::cerr << "Warning: Font not loaded in GameScene" << std::endl;
            return;
        }

        const sf::Font& font = FontManager::getInstance().getFont();

        // 初始化玩家1的棋子数量
        player1PieceCounts = {
            {"Q", 1},  // Queen
            {"A", 3},  // Ant
            {"S", 2},  // Spider
            {"B", 2},  // Beetle
            {"G", 3}   // Grasshopper
        };

        // 初始化玩家2的棋子数量（使用小写字母区分）
        player2PieceCounts = {
            {"q", 1},  // Queen
            {"a", 3},  // Ant
            {"s", 2},  // Spider
            {"b", 2},  // Beetle
            {"g", 3}   // Grasshopper
        };

        // 创建玩家1的初始棋子
        float startX = 50;
        float startY = 50;
        createPlayerPieces(player1Pieces, player1PieceCounts, font, startX, startY, true);

        // 创建玩家2的初始棋子
        startY = 600;
        createPlayerPieces(player2Pieces, player2PieceCounts, font, startX, startY, false);

        // 初始化棋盘格子位置
        initializeGridPositions();
    }
    void createPlayerPieces(std::vector<HexagonPiece>& pieces,
                      const std::map<std::string, int>& pieceCounts,
                      const sf::Font& font,
                      float startX, float startY,
                      bool isPlayer1) {
        float x = startX;
        for (const auto& [pieceType, count] : pieceCounts) {
            for (int i = 0; i < count; i++) {
                pieces.emplace_back(pieceType, font, sf::Vector2f(x, startY), isPlayer1);
                x += HEX_SIZE * 2.5f;
            }
        }
    }
    void initializeGridPositions() {
        // 创建蜂巢形状的棋盘格子位置
        const int GRID_SIZE = 11;  // 设置网格大小
        const float centerX = 512;  // 窗口中心X
        const float centerY = 384;  // 窗口中心Y

        for (int q = -GRID_SIZE; q <= GRID_SIZE; q++) {
            for (int r = -GRID_SIZE; r <= GRID_SIZE; r++) {
                // 使用轴坐标系统确保蜂巢形状
                if (abs(q + r) <= GRID_SIZE) {
                    float x = centerX + q * HORIZONTAL_SPACING + r * HORIZONTAL_SPACING / 2;
                    float y = centerY + r * VERTICAL_SPACING;
                    gridPositions.push_back({sf::Vector2f(x, y), false});
                }
            }
        }
    }

    void handleEvent(const sf::Event& event) override {
        sf::RenderWindow& window = SceneManager::getInstance().getWindow();

        std::vector<HexagonPiece>& currentPlayerPieces =
            isPlayer1Turn ? player1Pieces : player2Pieces;

        if (event.type == sf::Event::MouseButtonReleased) {
            sf::Vector2f mousePos = window.mapPixelToCoords(
                sf::Vector2i(event.mouseButton.x, event.mouseButton.y));

            // 检查是否有棋子被放置到棋盘上
            for (auto& piece : currentPlayerPieces) {
                if (piece.isDragging_()) {
                    // 寻找最近的有效格子
                    auto closestGrid = findClosestValidGrid(mousePos);
                    if (closestGrid != gridPositions.end()) {
                        piece.snapToPosition(closestGrid->first);
                        closestGrid->second = true;  // 标记该格子已被占用
                        switchTurn();
                    } else {
                        piece.resetPosition();  // 如果没有找到有效格子，返回原位置
                    }
                }
            }
        }

        // 处理正在拖动的棋子
        for (auto& piece : currentPlayerPieces) {
            piece.handleEvent(event, window);
        }
    }
    std::vector<std::pair<sf::Vector2f, bool>>::iterator findClosestValidGrid(const sf::Vector2f& pos) {
        float minDistance = std::numeric_limits<float>::max();
        auto closestGrid = gridPositions.end();

        for (auto it = gridPositions.begin(); it != gridPositions.end(); ++it) {
            if (!it->second) {  // 如果格子未被占用
                float distance = getDistance(pos, it->first);
                if (distance < minDistance && distance < HEX_SIZE) {
                    minDistance = distance;
                    closestGrid = it;
                }
            }
        }

        return closestGrid;
    }
    float getDistance(const sf::Vector2f& p1, const sf::Vector2f& p2) {
        float dx = p1.x - p2.x;
        float dy = p1.y - p2.y;
        return std::sqrt(dx * dx + dy * dy);
    }
    void switchTurn() {
        isPlayer1Turn = !isPlayer1Turn;
    }

    void update() override {
        // 可以在这里添加游戏逻辑更新
    }


    void render(sf::RenderWindow& window) override {
        // 绘制棋盘
        drawHexGrid(window);

        // 绘制所有棋子
        for (auto& piece : player1Pieces) {
            piece.draw(window);
        }
        for (auto& piece : player2Pieces) {
            piece.draw(window);
        }

        // 绘制剩余棋子数量
        drawPieceCounts(window);
    }

private:
    void drawHexGrid(sf::RenderWindow& window) {
        sf::CircleShape hexagon(HEX_SIZE, 6);
        hexagon.setFillColor(sf::Color::Transparent);
        hexagon.setOutlineThickness(1.f);
        hexagon.setOutlineColor(sf::Color(200, 200, 200));
        hexagon.setRotation(30.f);

        // 绘制所有格子
        for (const auto& [pos, occupied] : gridPositions) {
            hexagon.setPosition(pos);
            window.draw(hexagon);
        }
    }

    void drawPieceCounts(sf::RenderWindow& window) {
        if (!FontManager::getInstance().isLoaded()) return;

        const sf::Font& font = FontManager::getInstance().getFont();
        float startY1 = 10;  // 玩家1计数位置
        float startY2 = 700; // 玩家2计数位置
        float startX = 800;
        float spacing = 60;

        // 绘制玩家1的计数
        sf::Text text1;
        text1.setFont(font);
        text1.setCharacterSize(16);
        text1.setFillColor(sf::Color::Black);

        float x = startX;
        for (const auto& [piece, count] : player1PieceCounts) {
            text1.setString(piece + ": " + std::to_string(count));
            text1.setPosition(x, startY1);
            window.draw(text1);
            x += spacing;
        }

        // 绘制玩家2的计数
        sf::Text text2;
        text2.setFont(font);
        text2.setCharacterSize(16);
        text2.setFillColor(sf::Color::Black);

        x = startX;
        for (const auto& [piece, count] : player2PieceCounts) {
            text2.setString(piece + ": " + std::to_string(count));
            text2.setPosition(x, startY2);
            window.draw(text2);
            x += spacing;
        }

        // 显示当前回合玩家
        sf::Text turnText;
        turnText.setFont(font);
        turnText.setCharacterSize(24);
        turnText.setFillColor(sf::Color::Black);
        turnText.setString("Current Turn: Player " + std::string(isPlayer1Turn ? "1" : "2"));
        turnText.setPosition(400, 10);
        window.draw(turnText);
    }
};

void SceneManager::pushScene(SceneType type) {
    switch (type) {
        case SceneType::START:
            scenes.push(std::make_unique<StartScene>());
            break;
        case SceneType::MODE:
            scenes.push(std::make_unique<ModeScene>());
            break;
        case SceneType::GAME:
            scenes.push(std::make_unique<GameScene>());
            break;
    }
}

#endif // SCENE_MANAGER_HPP