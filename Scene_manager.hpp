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
    static constexpr float HEX_SIZE = 25.0f;
    sf::CircleShape hexagon;
    sf::Text text;
    sf::Vector2f position;
    sf::Vector2f originalPosition;
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
        hexagon.setOrigin(HEX_SIZE, HEX_SIZE);
        hexagon.setPosition(pos.x + HEX_SIZE, pos.y + HEX_SIZE);
        hexagon.setFillColor(isPlayer1 ? sf::Color(230, 230, 230) : sf::Color(200, 200, 200));
        hexagon.setOutlineThickness(2.f);
        hexagon.setOutlineColor(sf::Color::Black);

        text.setFont(font);
        text.setString(letter);
        text.setCharacterSize(20);
        text.setFillColor(sf::Color::Black);

        sf::FloatRect textBounds = text.getLocalBounds();
        text.setOrigin(textBounds.width/2, textBounds.height/2);
        text.setPosition(pos.x + HEX_SIZE, pos.y + HEX_SIZE);
    }
    const sf::Vector2f & getPosition()const{return position;}const
    void setisDragging(bool a){isDragging = a;}
    sf::Text getText(){return text;}
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
    bool snapToGrid(const sf::Vector2f& mousePos, const std::vector<std::pair<sf::Vector2f, bool>>& gridPositions) {
        float minDistance = std::numeric_limits<float>::max();
        auto closestGrid = gridPositions.end();
        for (auto it = gridPositions.begin(); it != gridPositions.end(); ++it) {
            if (!it->second) { // 如果格子未被占用
                float distance = getDistance(mousePos, it->first);
                if (distance < minDistance && distance < HEX_SIZE) {
                    minDistance = distance;
                    closestGrid = it;
                }
            }
        }
        if (closestGrid != gridPositions.end()) {
            position = closestGrid->first;
            updatePosition();
            return true;
        }
        return false;
    }
    static float getDistance(const sf::Vector2f& p1, const sf::Vector2f& p2) {
        float dx = p1.x - p2.x;
        float dy = p1.y - p2.y;
        return std::sqrt(dx * dx + dy * dy);
    }
    void resetPosition() {
        position = originalPosition;
        updatePosition();
    }
    void updatePosition() {
        hexagon.setPosition(position.x + HEX_SIZE, position.y + HEX_SIZE);
        sf::FloatRect textBounds = text.getLocalBounds();
        text.setOrigin(textBounds.width/2, textBounds.height/2);
        text.setPosition(position.x + HEX_SIZE, position.y + HEX_SIZE);
    }
    void setPosition(const sf::Vector2f& pos) {
        position = pos;
        updatePosition();
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
    // 统一棋子和棋盘格子大小
    static constexpr float HEX_SIZE = 25.0f;
    static constexpr float HEX_WIDTH = HEX_SIZE * 2.0f;
    static constexpr float HEX_HEIGHT = HEX_SIZE * sqrt(3.0f);
    static constexpr float HORIZONTAL_SPACING = HEX_WIDTH * 0.75f;
    static constexpr float VERTICAL_SPACING = HEX_HEIGHT;
    static constexpr int GRID_SIZE = 5;

    struct PieceState {
        piecetype::PieceName type;
        piecetype::PlayerID playerID;
        piecetype::HexCoord boardPosition;
        sf::Vector2f screenPosition;
        bool isPlaced;
        bool canBeMoved;
    };

    struct HexagonCell {
        sf::Vector2f position;
        piecetype::HexCoord coord;
        bool isOccupied;
        std::vector<PieceState*> pieces;
        bool isValidPlacement;
        piecetype::PlayerID occupiedBy; // 新增：记录占据格子的玩家
    };

    std::vector<HexagonPiece> player1Pieces;
    std::vector<HexagonPiece> player2Pieces;
    std::map<std::string, int> player1PieceCounts;
    std::map<std::string, int> player2PieceCounts;
    piecetype::Game game;
    bool isPlayer1Turn = true;
    bool isFirstPiecePlaced = false;
    HexagonPiece* selectedPiece = nullptr;
    sf::Vector2f originalPosition;
    std::vector<HexagonCell> hexGrid;
    std::map<HexagonPiece*, PieceState> pieceStates;
    // 存储棋盘格子的位置
    std::vector<std::pair<sf::Vector2f, bool>> gridPositions;
public:
    GameScene() {
        initializeGame();
        initializeGrid();
        createPieces();
    }
    void initializeGrid() {
        const float centerX = 512.0f;
        const float centerY = 384.0f;
        hexGrid.clear();

        for (int q = -GRID_SIZE; q <= GRID_SIZE; q++) {
            int r1 = std::max(-GRID_SIZE, -q - GRID_SIZE);
            int r2 = std::min(GRID_SIZE, -q + GRID_SIZE);
            for (int r = r1; r <= r2; r++) {
                // 修正坐标计算
                float x = centerX + (q * HORIZONTAL_SPACING);
                float y = centerY + (r * VERTICAL_SPACING + (q * VERTICAL_SPACING * 0.5f));

                HexagonCell cell;
                cell.position = sf::Vector2f(x, y);
                cell.coord = piecetype::HexCoord(q, r);
                cell.isOccupied = false;
                cell.isValidPlacement = true;
                cell.occupiedBy = piecetype::PlayerID::playernobody;
                hexGrid.push_back(cell);
            }
        }
    }
    // GameScene 中的创建棋子函数
    void createPieces() {
        if (!FontManager::getInstance().isLoaded()) return;
        const sf::Font& font = FontManager::getInstance().getFont();

        // 创建玩家1的棋子（上方）
        float startY1 = 50;
        float pieceSpacing = HEX_SIZE * 3.0f;

        // 计算每个玩家的总棋子数
        int totalPieces = 0;
        for (const auto& [_, count] : player1PieceCounts) {
            totalPieces += count;
        }

        // 计算开始位置，使棋子居中
        float startX1 = (1024 - (pieceSpacing * (totalPieces - 1))) / 2;
        float currentX = startX1;

        // 创建玩家1棋子
        for (const auto& [pieceType, count] : player1PieceCounts) {
            for (int i = 0; i < count; i++) {
                sf::Vector2f piecePos(currentX, startY1);
                player1Pieces.emplace_back(pieceType, font, piecePos, true);

                // 更新棋子状态
                PieceState state;
                state.type = getPieceTypeFromLetter(pieceType);
                state.playerID = piecetype::PlayerID::player1;
                state.isPlaced = false;
                state.screenPosition = piecePos;
                pieceStates[&player1Pieces.back()] = state;

                currentX += pieceSpacing;
            }
        }

        // 创建玩家2的棋子（下方）
        float startY2 = 650;
        float currentX2 = startX1;  // 使用相同的水平起始位置以保持对齐

        // 创建玩家2棋子
        for (const auto& [pieceType, count] : player2PieceCounts) {
            for (int i = 0; i < count; i++) {
                sf::Vector2f piecePos(currentX2, startY2);
                player2Pieces.emplace_back(pieceType, font, piecePos, false);

                // 更新棋子状态
                PieceState state;
                state.type = getPieceTypeFromLetter(pieceType);
                state.playerID = piecetype::PlayerID::player2;
                state.isPlaced = false;
                state.screenPosition = piecePos;
                pieceStates[&player2Pieces.back()] = state;

                currentX2 += pieceSpacing;
            }
        }
    }
    bool isValidMove(const HexagonPiece* piece, const HexagonCell& targetCell) {
        if (!piece) return false;
        auto it = pieceStates.find(const_cast<HexagonPiece*>(piece));
        if (it == pieceStates.end()) return false;

        const PieceState& state = it->second;

        // 检查是否是当前玩家的回合
        bool isCurrentPlayerPiece = (isPlayer1Turn && state.playerID == piecetype::PlayerID::player1) ||
                                  (!isPlayer1Turn && state.playerID == piecetype::PlayerID::player2);
        if (!isCurrentPlayerPiece) return false;

        // 第一个棋子的放置规则
        if (!isFirstPiecePlaced) {
            if (game.getBoard().getTotalPieces() == 0) {
                return true;
            }
            return false;
        }

        // 检查第二个玩家的放置规则
        if (!state.isPlaced) {
            if (!validateNewPlacement(state, targetCell)) return false;

            // 确保第二个玩家的棋子放置位置与第一个玩家的棋子相邻
            if (game.getBoard().getTotalPieces() == 1) {
                bool hasAdjacentPiece = false;
                for (const auto& neighbor : targetCell.coord.neighbors()) {
                    for (const auto& cell : hexGrid) {
                        if (cell.coord == neighbor && cell.isOccupied) {
                            hasAdjacentPiece = true;
                            break;
                        }
                    }
                }
                return hasAdjacentPiece;
            }
        }

        return validatePieceMove(state, targetCell);
    }
    // 初始化所有棋子的状态
    void initializePieceStates() {
        // 初始化玩家1棋子状态
        for (auto& piece : player1Pieces) {
            PieceState state;
            state.playerID = piecetype::PlayerID::player1;
            state.isPlaced = false;
            state.canBeMoved = true;
            state.screenPosition = piece.getPosition();
            state.type = pieceStates[&piece].type;
            pieceStates[&piece] = state;
        }

        // 初始化玩家2棋子状态
        for (auto& piece : player2Pieces) {
            PieceState state;
            state.playerID = piecetype::PlayerID::player2;
            state.isPlaced = false;
            state.canBeMoved = true;
            state.screenPosition = piece.getPosition();
            state.type = pieceStates[&piece].type;
            pieceStates[&piece] = state;
        }
    }
    // 根据字母获取棋子类型
    piecetype::PieceName getPieceTypeFromLetter(const std::string& letter) {
        char upperLetter = std::toupper(letter[0]);
        switch (upperLetter) {
            case 'Q': return piecetype::PieceName::Queen;    // 蜂后
            case 'A': return piecetype::PieceName::Ant;      // 蚂蚁
            case 'S': return piecetype::PieceName::Spider;   // 蜘蛛
            case 'B': return piecetype::PieceName::Beetle;   // 甲虫
            case 'G': return piecetype::PieceName::Grasshopper; // 蚱蜢
            default: throw std::runtime_error("未知的棋子类型");
        }
    }
    // 找到指定位置的棋子
    HexagonPiece* findPieceAtPosition(const sf::Vector2f& pos) {
        // 检查玩家1的棋子
        for (auto& piece : player1Pieces) {
            sf::Vector2f piecePos = piece.getPosition();
            float distance = std::sqrt(
                std::pow(pos.x - piecePos.x - HEX_SIZE, 2) +
                std::pow(pos.y - piecePos.y - HEX_SIZE, 2)
            );
            if (distance < HEX_SIZE) return &piece;
        }

        // 检查玩家2的棋子
        for (auto& piece : player2Pieces) {
            sf::Vector2f piecePos = piece.getPosition();
            float distance = std::sqrt(
                std::pow(pos.x - piecePos.x - HEX_SIZE, 2) +
                std::pow(pos.y - piecePos.y - HEX_SIZE, 2)
            );
            if (distance < HEX_SIZE) return &piece;
        }

        return nullptr;
    }
    HexagonCell* findNearestCell(const sf::Vector2f& mousePos) {
        HexagonCell* nearest = nullptr;
        float minDistance = std::numeric_limits<float>::max();

        for (auto& cell : hexGrid) {
            float dx = mousePos.x - cell.position.x;
            float dy = mousePos.y - cell.position.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance < minDistance && distance < HEX_SIZE * 1.5f) {
                minDistance = distance;
                nearest = &cell;
            }
        }

        return nearest;
    }
    // 创建特定类型的棋子
    std::shared_ptr<piecetype::Piece> createPieceForType(
        piecetype::PieceName type, piecetype::PlayerID playerID) {
        switch (type) {
            case piecetype::PieceName::Queen:
                return std::make_shared<piecetype::QueenBee>(playerID);
            case piecetype::PieceName::Ant:
                return std::make_shared<piecetype::Ant>(playerID);
            case piecetype::PieceName::Spider:
                return std::make_shared<piecetype::Spider>(playerID);
            case piecetype::PieceName::Beetle:
                return std::make_shared<piecetype::Beetle>(playerID);
            case piecetype::PieceName::Grasshopper:
                return std::make_shared<piecetype::Grasshopper>(playerID);
            default:
                throw std::runtime_error("未知的棋子类型");
        }
    }
    // 重置棋子位置
    void resetPiece() {
        if (!selectedPiece) return;
        selectedPiece->snapToPosition(originalPosition);
    }


    void initializeGame() {
        // 初始化游戏状态
        isPlayer1Turn = true;
        game = piecetype::Game();  // 确保游戏对象被正确初始化
        // 初始化游戏状态和计数器
        player1PieceCounts = {{"Q", 1}, {"A", 3}, {"S", 2}, {"B", 2}, {"G", 3}};
        player2PieceCounts = {{"q", 1}, {"a", 3}, {"s", 2}, {"b", 2}, {"g", 3}};
    }


    bool validateNewPlacement(const PieceState& state, const HexagonCell& targetCell) {
        // 第一个棋子可以放在任何位置
        if (game.getBoard().getTotalPieces() == 0) return true;

        return game.getBoard().canPlacePiece(targetCell.coord,
            isPlayer1Turn ? piecetype::PlayerID::player1 : piecetype::PlayerID::player2);
    }

    bool validatePieceMove(const PieceState& state, const HexagonCell& targetCell) {
        // 检查连续性
        if (!game.getBoard().willMoveMaintainContinuity(state.boardPosition, targetCell.coord)) {
            return false;
        }

        // 获取棋子对象
        auto piece = game.getBoard().getPieceAt(state.boardPosition);
        if (!piece) return false;

        // 检查是否是"眼"位置
        if (game.getBoard().isEye(targetCell.coord)) {
            return state.type == piecetype::PieceName::Grasshopper;
        }

        // 验证具体棋子类型的移动规则
        return piece->isValidMove(targetCell.coord, game.getBoard());
    }

    void handlePieceSelection(const sf::Vector2f& mousePos) {
        // 找出被点击的棋子
        HexagonPiece* clickedPiece = findPieceAtPosition(mousePos);
        if (!clickedPiece) return;

        auto it = pieceStates.find(clickedPiece);
        if (it == pieceStates.end()) return;

        // 检查是否是当前玩家的棋子
        if ((isPlayer1Turn && it->second.playerID != piecetype::PlayerID::player1) ||
            (!isPlayer1Turn && it->second.playerID != piecetype::PlayerID::player2)) {
            return;
        }

        // 检查是否已放置且移动是否会破坏连续性
        if (it->second.isPlaced) {
            if (!it->second.canBeMoved ||
                !game.getBoard().willMoveMaintainContinuity(it->second.boardPosition,
                                                          it->second.boardPosition)) {
                return;
            }
        }

        selectedPiece = clickedPiece;
        originalPosition = clickedPiece->getPosition();
    }

    void handlePiecePlacement(sf::Vector2f mousePos) {
        if (!selectedPiece) return;

        // 寻找最近的有效格子
        HexagonCell* targetCell = nullptr;
        float minDistance = std::numeric_limits<float>::max();

        for (auto& cell : hexGrid) {
            float distance = std::sqrt(
                std::pow(mousePos.x - cell.position.x, 2) +
                std::pow(mousePos.y - cell.position.y, 2)
            );

            if (distance < minDistance && distance < HEX_SIZE * 1.5f) {
                minDistance = distance;
                targetCell = &cell;
            }
        }

        if (targetCell && isValidMove(selectedPiece, *targetCell)) {
            placePiece(selectedPiece, *targetCell);
            if (!isFirstPiecePlaced) {
                isFirstPiecePlaced = true;
            }
        } else {
            selectedPiece->resetPosition();
        }

        selectedPiece = nullptr;
    }
    void completePieceMovement(HexagonCell& targetCell) {
        auto& state = pieceStates[selectedPiece];

        // 更新游戏状态
        if (!state.isPlaced) {
            // 新棋子放置
            auto piece = createPieceForType(state.type, state.playerID);
            game.getBoard().addPiece(piece, targetCell.coord, state.playerID);
            state.isPlaced = true;
        } else {
            // 移动已存在的棋子
            auto piece = game.getBoard().getPieceAt(state.boardPosition);
            if (piece) {
                piece->move(game.getBoard(), targetCell.coord, state.playerID);
            }
        }

        // 更新棋子状态
        state.boardPosition = targetCell.coord;
        state.screenPosition = targetCell.position;
        selectedPiece->snapToPosition(targetCell.position);

        // 更新格子状态
        targetCell.isOccupied = true;
        targetCell.pieces.push_back(&state);

        // 切换玩家
        switchTurn();
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
    void placePiece(HexagonPiece* piece, HexagonCell& targetCell) {
        auto& state = pieceStates[piece];

        if (!state.isPlaced) {
            auto newPiece = createPieceForType(state.type, state.playerID);
            try {
                game.getBoard().addPiece(newPiece, targetCell.coord, state.playerID);
                state.isPlaced = true;
                state.boardPosition = targetCell.coord;
                targetCell.isOccupied = true;
                targetCell.occupiedBy = state.playerID;

                if (!isFirstPiecePlaced) {
                    isFirstPiecePlaced = true;
                }

                // 修正棋子放置位置
                piece->setPosition(sf::Vector2f(
                    targetCell.position.x - HEX_SIZE,
                    targetCell.position.y - HEX_SIZE
                ));
            } catch (const std::exception& e) {
                piece->resetPosition();
                std::cerr << "Failed to place piece: " << e.what() << std::endl;
            }
        } else {
            try {
                auto gamePiece = game.getBoard().getPieceAt(state.boardPosition);
                if (gamePiece) {
                    gamePiece->move(game.getBoard(), targetCell.coord, state.playerID);
                    state.boardPosition = targetCell.coord;
                    // 修正棋子移动位置
                    piece->setPosition(sf::Vector2f(
                        targetCell.position.x - HEX_SIZE,
                        targetCell.position.y - HEX_SIZE
                    ));
                }
            } catch (const std::exception& e) {
                piece->resetPosition();
                std::cerr << "Failed to move piece: " << e.what() << std::endl;
            }
        }
    }

    void handleEvent(const sf::Event& event) override {
        sf::RenderWindow& window = SceneManager::getInstance().getWindow();

        if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f mousePos = window.mapPixelToCoords(
                sf::Vector2i(event.mouseButton.x, event.mouseButton.y));

            if (!selectedPiece) {
                auto clickedPiece = findPieceAtPosition(mousePos);
                if (clickedPiece) {
                    auto it = pieceStates.find(clickedPiece);
                    if (it != pieceStates.end()) {
                        // 只允许当前回合玩家选择自己的棋子
                        bool isCurrentPlayerPiece = (isPlayer1Turn && it->second.playerID == piecetype::PlayerID::player1) ||
                                                  (!isPlayer1Turn && it->second.playerID == piecetype::PlayerID::player2);
                        if (isCurrentPlayerPiece) {
                            selectedPiece = clickedPiece;
                            originalPosition = clickedPiece->getPosition();
                            clickedPiece->setisDragging(true);
                        }
                    }
                }
            }
        }
        else if (event.type == sf::Event::MouseButtonReleased) {
            if (selectedPiece) {
                sf::Vector2f mousePos = window.mapPixelToCoords(
                    sf::Vector2i(event.mouseButton.x, event.mouseButton.y));

                HexagonCell* targetCell = findNearestCell(mousePos);
                if (targetCell && isValidMove(selectedPiece, *targetCell)) {
                    placePiece(selectedPiece, *targetCell);
                    switchTurn();
                } else {
                    selectedPiece->resetPosition();
                }
                selectedPiece->setisDragging(false);
                selectedPiece = nullptr;
            }
        }
        else if (event.type == sf::Event::MouseMoved) {
            if (selectedPiece && selectedPiece->isDragging_()) {
                sf::Vector2f mousePos = window.mapPixelToCoords(
                    sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
                selectedPiece->setPosition(mousePos);
                selectedPiece->updatePosition();
            }
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
    static float getDistance(const sf::Vector2f& p1, const sf::Vector2f& p2) {
        float dx = p1.x - p2.x;
        float dy = p1.y - p2.y;
        return std::sqrt(dx * dx + dy * dy);
    }
    void switchTurn() {
        isPlayer1Turn = !isPlayer1Turn;
    }

    // 更新游戏状态
    void update() override {
        // 检查胜利条件
        piecetype::Victory victory = game.getBoard().checkVictory();
        if (victory != piecetype::Victory::NONE) {
            handleGameOver(victory);
        }
    }
    // 处理游戏结束
    void handleGameOver(piecetype::Victory victory) {
        std::string message;
        switch (victory) {
            case piecetype::Victory::PLAYER1_WINS:
                message = "玩家1获胜！";
            break;
            case piecetype::Victory::PLAYER2_WINS:
                message = "玩家2获胜！";
            break;
            case piecetype::Victory::DRAW:
                message = "平局！";
            break;
            default:
                return;
        }

        // 创建并显示胜利文本
        if (FontManager::getInstance().isLoaded()) {
            sf::Text victoryText;
            victoryText.setFont(FontManager::getInstance().getFont());
            victoryText.setString(message);
            victoryText.setCharacterSize(48);
            victoryText.setFillColor(sf::Color::Red);
            victoryText.setPosition(400, 300);
        }
    }

    void render(sf::RenderWindow& window) override {
        window.clear(sf::Color::White);

        drawHexGrid(window);

        // 绘制吸附提示
        if (selectedPiece) {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            HexagonCell* targetCell = findNearestCell(mousePos);
            if (targetCell && isValidMove(selectedPiece, *targetCell)) {
                drawSnapPreview(window, *targetCell);
            }
        }

        // 绘制所有棋子
        for (auto& piece : player1Pieces) {
            piece.draw(window);
        }
        for (auto& piece : player2Pieces) {
            piece.draw(window);
        }

        drawTurnIndicator(window);
        drawPieceCounts(window);
    }

private:
    void drawSnapPreview(sf::RenderWindow& window, const HexagonCell& cell) {
        sf::CircleShape preview(HEX_SIZE, 6);
        preview.setRotation(30.f);
        preview.setPosition(cell.position);
        preview.setFillColor(sf::Color(100, 255, 100, 100));
        preview.setOutlineThickness(2.f);
        preview.setOutlineColor(sf::Color::Green);
        window.draw(preview);
    }
    // 绘制回合指示器
    void drawTurnIndicator(sf::RenderWindow& window) {
        if (!FontManager::getInstance().isLoaded()) return;

        sf::Text turnText;
        turnText.setFont(FontManager::getInstance().getFont());
        turnText.setCharacterSize(24);
        turnText.setFillColor(sf::Color::Black);
        turnText.setPosition(412, 10);

        std::string turnString = "Current Turn: Player " + std::string(isPlayer1Turn ? "1" : "2");
        turnText.setString(turnString);
        window.draw(turnText);
    }
    void drawHexGrid(sf::RenderWindow& window) {
        sf::CircleShape hexagon(HEX_SIZE, 6);
        hexagon.setRotation(30.f);
        hexagon.setOutlineThickness(1.f);
        hexagon.setOutlineColor(sf::Color(100, 100, 100));
        hexagon.setOrigin(HEX_SIZE, HEX_SIZE); // 设置原点在中心

        for (const auto& cell : hexGrid) {
            // 修正渲染位置
            hexagon.setPosition(cell.position);

            if (cell.isOccupied) {
                hexagon.setFillColor(sf::Color(220, 220, 220));
            } else {
                hexagon.setFillColor(sf::Color(245, 245, 245));
            }

            window.draw(hexagon);
        }
    }


    // 绘制棋子数量显示
    void drawPieceCounts(sf::RenderWindow& window) {
        if (!FontManager::getInstance().isLoaded()) return;

        const sf::Font& font = FontManager::getInstance().getFont();
        float startY1 = 10;  // 玩家1计数位置
        float startY2 = 700; // 玩家2计数位置
        float startX = 800;
        float spacing = 60;

        // 绘制玩家1的剩余棋子数量
        sf::Text text1;
        text1.setFont(font);
        text1.setCharacterSize(16);
        text1.setFillColor(sf::Color::Black);

        for (const auto& [piece, count] : player1PieceCounts) {
            std::string pieceName;
            switch(piece[0]) {
                case 'Q': pieceName = "蜂后"; break;
                case 'A': pieceName = "蚂蚁"; break;
                case 'S': pieceName = "蜘蛛"; break;
                case 'B': pieceName = "甲虫"; break;
                case 'G': pieceName = "蚱蜢"; break;
                default: pieceName = piece;
            }
            text1.setString(pieceName + ": " + std::to_string(count));
            text1.setPosition(startX, startY1);
            window.draw(text1);
            startX += spacing;
        }

        // 重置X坐标以绘制玩家2的计数
        startX = 800;
        sf::Text text2;
        text2.setFont(font);
        text2.setCharacterSize(16);
        text2.setFillColor(sf::Color::Black);

        for (const auto& [piece, count] : player2PieceCounts) {
            std::string pieceName;
            switch(std::toupper(piece[0])) {
                case 'Q': pieceName = "蜂后"; break;
                case 'A': pieceName = "蚂蚁"; break;
                case 'S': pieceName = "蜘蛛"; break;
                case 'B': pieceName = "甲虫"; break;
                case 'G': pieceName = "蚱蜢"; break;
                default: pieceName = piece;
            }
            text2.setString(pieceName + ": " + std::to_string(count));
            text2.setPosition(startX, startY2);
            window.draw(text2);
            startX += spacing;
        }
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