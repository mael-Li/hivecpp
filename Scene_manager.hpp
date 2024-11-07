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
        : position(pos), originalPosition(pos), isPlayer1(isPlayer1){

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
    void resetPosition() {
        position = originalPosition;
        updatePosition();
    }
    void updatePosition() {
        hexagon.setPosition(position);
        text.setPosition(position.x + HEX_SIZE, position.y + HEX_SIZE);
    }
    void setPosition(sf::Vector2f& pos){position = pos;}
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
    // 新增：跟踪棋子状态和位置的数据结构
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
        std::vector<PieceState*> pieces;  // 支持堆叠
    };
    // UI 元素
    std::vector<HexagonPiece> player1Pieces;
    std::vector<HexagonPiece> player2Pieces;
    std::map<std::string, int> player1PieceCounts;
    std::map<std::string, int> player2PieceCounts;

    piecetype::Game game;
    bool isPlayer1Turn = true;  // 当前回合玩家
    // 存储棋盘格子的位置
    std::vector<std::pair<sf::Vector2f, bool>> gridPositions;
    // 调整六边形大小和间距
    static constexpr float HEX_SIZE = 30.0f; // 六边形大小
    static constexpr float HEX_WIDTH = HEX_SIZE * 2.0f; // 六边形宽度
    static constexpr float HEX_HEIGHT = HEX_SIZE * sqrt(3.0f); // 六边形高度
    static constexpr float HORIZONTAL_SPACING = HEX_WIDTH * 0.75f; // 水平间距
    static constexpr float VERTICAL_SPACING = HEX_HEIGHT; // 垂直间距
    std::map<HexagonPiece*, PieceState> pieceStates;
    std::vector<HexagonCell> hexGrid;
    HexagonPiece* selectedPiece = nullptr;
    sf::Vector2f originalPosition;


public:
    GameScene() {
        initializeGame();
        initializeGrid();
        createPieces();
    }
    // 创建所有游戏棋子
    void createPieces() {
        if (!FontManager::getInstance().isLoaded()) return;
        const sf::Font& font = FontManager::getInstance().getFont();

        // 创建玩家1的棋子（上方）
        float startY1 = 50;
        float startX1 = 100;
        createPlayerPieces(player1Pieces, player1PieceCounts, font, startX1, startY1, true);

        // 创建玩家2的棋子（下方）
        float startY2 = 650;
        float startX2 = 100;
        createPlayerPieces(player2Pieces, player2PieceCounts, font, startX2, startY2, false);

        // 初始化棋子状态
        initializePieceStates();
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
    // 找到最近的格子
    HexagonCell* findNearestCell(const sf::Vector2f& pos) {
        HexagonCell* nearest = nullptr;
        float minDistance = std::numeric_limits<float>::max();

        for (auto& cell : hexGrid) {
            float distance = std::sqrt(
                std::pow(pos.x - cell.position.x - HEX_SIZE, 2) +
                std::pow(pos.y - cell.position.y - HEX_SIZE, 2)
            );

            if (distance < minDistance && distance < HEX_SIZE * 1.5) {
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
    void initializeGrid() {
        const int GRID_SIZE = 8; // 调整网格大小
        const float centerX = 512.0f; // 窗口中心X
        const float centerY = 384.0f; // 窗口中心Y

        // 清空现有网格
        hexGrid.clear();
        gridPositions.clear();

        // 创建六边形网格
        for (int row = -GRID_SIZE; row <= GRID_SIZE; ++row) {
            int rowOffset = floor(row/2.0f); // 计算行偏移
            for (int col = -GRID_SIZE-rowOffset; col <= GRID_SIZE-rowOffset; ++col) {
                // 计算六边形的坐标
                float x = centerX + col * HEX_WIDTH + (row % 2) * HEX_WIDTH/2;
                float y = centerY + row * HEX_HEIGHT * 0.75f;

                // 创建并存储六边形单元格
                HexagonCell cell;
                cell.position = sf::Vector2f(x, y);
                cell.coord = piecetype::HexCoord(col, row);
                cell.isOccupied = false;
                hexGrid.push_back(cell);

                // 同时存储到 gridPositions
                gridPositions.push_back({sf::Vector2f(x, y), false});
            }
        }
    }


    bool isValidMove(const HexagonPiece* piece, const HexagonCell& targetCell) {
        auto it = pieceStates.find(const_cast<HexagonPiece*>(piece));
        if (it == pieceStates.end()) return false;

        const PieceState& state = it->second;

        // 如果是新棋子放置
        if (!state.isPlaced) {
            return validateNewPlacement(state, targetCell);
        }

        // 如果是移动已放置的棋子
        return validatePieceMove(state, targetCell);
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

    void handlePiecePlacement(const sf::Vector2f& mousePos) {
        if (!selectedPiece) return;

        // 找到最近的有效格子
        HexagonCell* targetCell = findNearestCell(mousePos);
        if (!targetCell) {
            resetPiece();
            return;
        }

        // 验证移动
        if (isValidMove(selectedPiece, *targetCell)) {
            completePieceMovement(*targetCell);
        } else {
            resetPiece();
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
            // 新棋子放置
            auto newPiece = createPieceForType(state.type, state.playerID);
            try {
                game.getBoard().addPiece(newPiece, targetCell.coord, state.playerID);
                state.isPlaced = true;
                state.boardPosition = targetCell.coord;
                targetCell.isOccupied = true;

                // 更新计数器
                if (state.playerID == piecetype::PlayerID::player1) {
                    player1PieceCounts[piece->getText().getString()]--;
                } else {
                    player2PieceCounts[piece->getText().getString()]--;
                }

                piece->snapToPosition(targetCell.position);
            } catch (const std::exception& e) {
                piece->resetPosition();
                std::cerr << "Failed to place piece: " << e.what() << std::endl;
            }
        } else {
            // 移动已存在的棋子
            try {
                auto gamePiece = game.getBoard().getPieceAt(state.boardPosition);
                if (gamePiece) {
                    gamePiece->move(game.getBoard(), targetCell.coord, state.playerID);
                    state.boardPosition = targetCell.coord;
                    piece->snapToPosition(targetCell.position);
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

        // 只有当没有选中棋子时才尝试选择新棋子
        if (!selectedPiece) {
            auto clickedPiece = findPieceAtPosition(mousePos);
            if (clickedPiece && ((isPlayer1Turn && pieceStates[clickedPiece].playerID == piecetype::PlayerID::player1) ||
                                (!isPlayer1Turn && pieceStates[clickedPiece].playerID == piecetype::PlayerID::player2))) {
                selectedPiece = clickedPiece;
                originalPosition = clickedPiece->getPosition();
                clickedPiece->setisDragging(true);
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
        }
    }

    // 让所有棋子处理事件
    for (auto& piece : player1Pieces) {
        piece.handleEvent(event, window);
    }
    for (auto& piece : player2Pieces) {
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

    // 渲染游戏画面
    void render(sf::RenderWindow& window) override {
        window.clear(sf::Color::White);
        drawHexGrid(window);

        // 绘制所有棋子
        for (auto& piece : player1Pieces) {
            piece.draw(window);
        }
        for (auto& piece : player2Pieces) {
            piece.draw(window);
        }

        // 添加调试信息
        if (selectedPiece) {
            sf::CircleShape highlight(HEX_SIZE);
            highlight.setFillColor(sf::Color(255, 255, 0, 128));
            highlight.setPosition(selectedPiece->getPosition());
            window.draw(highlight);
        }

        drawPieceCounts(window);
    }
private:
    // 绘制回合指示器
    void drawTurnIndicator(sf::RenderWindow& window) {
        if (!FontManager::getInstance().isLoaded()) return;

        sf::Text turnText;
        turnText.setFont(FontManager::getInstance().getFont());
        turnText.setCharacterSize(24);
        turnText.setFillColor(sf::Color::Black);
        turnText.setString("当前回合: 玩家" + std::string(isPlayer1Turn ? "1" : "2"));
        turnText.setPosition(400, 10);
        window.draw(turnText);
    }
    void drawHexGrid(sf::RenderWindow& window) {
        // 创建基础六边形形状
        sf::CircleShape hexagon(HEX_SIZE, 6);
        hexagon.setRotation(30.f);
        hexagon.setFillColor(sf::Color(245, 245, 245)); // 浅灰色填充
        hexagon.setOutlineThickness(1.f);
        hexagon.setOutlineColor(sf::Color(100, 100, 100)); // 深灰色边框

        // 设置六边形的原点为其中心
        hexagon.setOrigin(HEX_SIZE, HEX_SIZE);

        // 绘制所有格子
        for (const auto& cell : hexGrid) {
            hexagon.setPosition(cell.position);

            // 如果格子被占用，使用不同的颜色
            if (cell.isOccupied) {
                hexagon.setFillColor(sf::Color(200, 200, 200));
            } else {
                hexagon.setFillColor(sf::Color(245, 245, 245));
            }

            window.draw(hexagon);

            // 可选：绘制调试坐标
            /*if (FontManager::getInstance().isLoaded()) {
                sf::Text coordText;
                coordText.setFont(FontManager::getInstance().getFont());
                coordText.setString("(" + std::to_string(cell.coord.q) + ","
                                     + std::to_string(cell.coord.r) + ")");
                coordText.setCharacterSize(8);
                coordText.setFillColor(sf::Color::Black);
                coordText.setPosition(cell.position);
                window.draw(coordText);
            }*/
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