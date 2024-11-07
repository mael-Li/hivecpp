//
// Created by 李世佳 on 24-11-8.
//

#ifndef HIVE_GUI_HPP
#define HIVE_GUI_HPP
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <cmath>

#include "GameConstants.h"
#include "Hive.h"
namespace piecetype {
    // 游戏相关枚举定义
    enum class GameState;
    enum class MenuOption;
    enum class AnimationType;
    // 动画状态结构体完整定义
    struct AnimationState {
        AnimationType type = AnimationType::NONE;
        float progress = 0.0f;
        sf::Vector2f startPos;
        sf::Vector2f endPos;
        float duration = 0.5f;

        AnimationState()
            : type(AnimationType::NONE), progress(0.0f),
              startPos(0.0f, 0.0f), endPos(0.0f, 0.0f), duration(0.5f) {}
    };
    class MenuSystem;
    class BoardComponent;
    class PieceComponent;
// GUI组件的基类
class GUIComponent {
protected:
    sf::RenderWindow& window;
    sf::Font font;

public:
    GUIComponent(sf::RenderWindow& win) : window(win) {
        if (!font.loadFromFile("arial.ttf")) {
            // 尝试加载系统字体
            #ifdef _WIN32
            font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");
            #else
            font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
            #endif
        }
    }
    virtual void draw() = 0;
    virtual void handleEvent(const sf::Event& event) = 0;
    virtual ~GUIComponent() = default;
};

// 主菜单组件
class MenuComponent : public GUIComponent {
private:
    sf::Text title;
    sf::RectangleShape startButton;
    sf::Text startText;
    bool& isGameStarted;

public:
    MenuComponent(sf::RenderWindow& win, bool& gameStarted)
        : GUIComponent(win), isGameStarted(gameStarted) {
        // 设置标题
        title.setFont(font);
        title.setString("Hive Game");
        title.setCharacterSize(50);
        title.setFillColor(sf::Color::Black);
        title.setPosition(400, 200);

        // 设置开始按钮
        startButton.setSize(sf::Vector2f(200, 50));
        startButton.setPosition(400, 300);
        startButton.setFillColor(sf::Color::Green);

        startText.setFont(font);
        startText.setString("Start Game");
        startText.setCharacterSize(30);
        startText.setFillColor(sf::Color::White);
        startText.setPosition(430, 310);
    }

    void draw() override {
        window.draw(title);
        window.draw(startButton);
        window.draw(startText);
    }

    void handleEvent(const sf::Event& event) override {
        if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            if (startButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                isGameStarted = true;
            }
        }
    }
};

// 游戏面板组件（显示玩家信息、回合等）
class GamePanelComponent : public GUIComponent {
private:
    const Board& board;
    sf::Text turnText;
    sf::Text p1Stats;
    sf::Text p2Stats;

public:
    GamePanelComponent(sf::RenderWindow& win, const Board& b)
        : GUIComponent(win), board(b) {
        // 设置回合信息文本
        turnText.setFont(font);
        turnText.setCharacterSize(20);
        turnText.setFillColor(sf::Color::Black);
        turnText.setPosition(10, 10);

        // 设置玩家1信息
        p1Stats.setFont(font);
        p1Stats.setCharacterSize(16);
        p1Stats.setFillColor(sf::Color::Black);
        p1Stats.setPosition(10, 40);

        // 设置玩家2信息
        p2Stats.setFont(font);
        p2Stats.setCharacterSize(16);
        p2Stats.setFillColor(sf::Color::Black);
        p2Stats.setPosition(10, 70);
    }

    void updateStats(int turn, bool isP1Turn) {
        turnText.setString("Turn: " + std::to_string(turn) +
                          " | Current Player: " + (isP1Turn ? "Player 1" : "Player 2"));

        // 更新玩家信息
        std::string p1Info = "Player 1 - Queens: " +
            std::to_string(board.piecesAvailable.at(PlayerID::player1).at(PieceName::Queen));
        std::string p2Info = "Player 2 - Queens: " +
            std::to_string(board.piecesAvailable.at(PlayerID::player2).at(PieceName::Queen));

        p1Stats.setString(p1Info);
        p2Stats.setString(p2Info);
    }

    void draw() override {
        window.draw(turnText);
        window.draw(p1Stats);
        window.draw(p2Stats);
    }

    void handleEvent(const sf::Event&) override {}
};

// 棋盘组件
//棋子组件类
class PieceComponent {
private:
    static constexpr float PIECE_SIZE = 25.0f;
    sf::CircleShape shape;
    sf::Text label;
    AnimationState animation;
    PlayerID player;
    PieceName type;
    bool isDragging = false;
    sf::Vector2f dragOffset;
    bool isPlaced = false;
    HexCoord boardPosition;
    float elevation = 0.0f; // 用于堆叠效果
    // 添加一个静态map来存储不同棋子类型的初始位置
    static const std::map<PieceName, sf::Vector2f> initialPiecePositions;
public:
    PieceComponent(const sf::Font& font, PieceName type, PlayerID player)
        : type(type), player(player) {
        // 设置棋子形状
        shape.setRadius(PIECE_SIZE);
        shape.setPointCount(6);
        shape.setRotation(30.f);
        shape.setOrigin(PIECE_SIZE, PIECE_SIZE);

        // 设置棋子颜色
        if (player == PlayerID::player1) {
            shape.setFillColor(sf::Color(230, 230, 230));
        } else {
            shape.setFillColor(sf::Color(200, 200, 200));
        }
        shape.setOutlineThickness(2.f);
        shape.setOutlineColor(sf::Color::Black);

        // 设置标签
        label.setFont(font);
        label.setCharacterSize(20);
        label.setFillColor(sf::Color::Black);
        label.setString(getPieceLabel());

        // 居中标签
        sf::FloatRect bounds = label.getLocalBounds();
        label.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    }

    std::string getPieceLabel() const {
        switch (type) {
            case PieceName::Queen: return "Q";
            case PieceName::Ant: return "A";
            case PieceName::Spider: return "S";
            case PieceName::Beetle: return "B";
            case PieceName::Grasshopper: return "G";
            case PieceName::Worm: return "W";
            default: return "?";
        }
    }

    void setPosition(const sf::Vector2f& pos) {
        shape.setPosition(pos);
        label.setPosition(pos);
    }

    void startAnimation(AnimationType type, const sf::Vector2f& start, const sf::Vector2f& end) {
        animation.type = type;
        animation.progress = 0.0f;
        animation.startPos = start;
        animation.endPos = end;
    }

    void updateAnimation(float deltaTime) {
        if (animation.type == AnimationType::NONE) return;

        animation.progress += deltaTime / animation.duration;
        if (animation.progress >= 1.0f) {
            animation.progress = 1.0f;
            animation.type = AnimationType::NONE;
            setPosition(animation.endPos);
        } else {
            float t = animation.progress;
            // 使用缓动函数使动画更平滑
            t = t * t * (3 - 2 * t); // 使用三次方缓动
            sf::Vector2f pos = animation.startPos + (animation.endPos - animation.startPos) * t;

            // 添加弧线效果（对于移动和放置动画）
            if (animation.type == AnimationType::MOVE || animation.type == AnimationType::PLACE) {
                float arc = std::sin(t * 3.14159f) * 30.0f; // 30像素的最大高度
                pos.y -= arc;
            }

            setPosition(pos);
        }
    }

    void startDrag(const sf::Vector2f& mousePos) {
        isDragging = true;
        dragOffset = shape.getPosition() - mousePos;
        elevation = 20.0f; // 提升高度表示正在拖动
    }

    void endDrag() {
        isDragging = false;
        elevation = 0.0f;
    }

    void updateDragPosition(const sf::Vector2f& mousePos) {
        if (isDragging) {
            setPosition(mousePos + dragOffset);
        }
    }

    void draw(sf::RenderWindow& window) {
        // 绘制阴影
        if (elevation > 0.0f) {
            sf::CircleShape shadow = shape;
            shadow.setFillColor(sf::Color(0, 0, 0, 50));
            shadow.setOutlineThickness(0);
            shadow.setPosition(shape.getPosition() + sf::Vector2f(5, 5));
            window.draw(shadow);
        }

        window.draw(shape);
        window.draw(label);
    }

    bool contains(const sf::Vector2f& point) const {
        return shape.getGlobalBounds().contains(point);
    }

    void setBoardPosition(const HexCoord& pos) {
        boardPosition = pos;
        isPlaced = true;
    }

    const HexCoord& getBoardPosition() const { return boardPosition; }
    bool isOnBoard() const { return isPlaced; }
    PlayerID getPlayer() const { return player; }
    PieceName getType() const { return type; }
    // 获取棋子的当前位置
    sf::Vector2f getPosition() const {
        return shape.getPosition();
    }
    // 获取特定棋子类型的初始位置
    static sf::Vector2f getInitialPosition(PieceName pieceType) {
        auto it = initialPiecePositions.find(pieceType);
        if (it != initialPiecePositions.end()) {
            return it->second;
        }
        return sf::Vector2f(0, 0); // 如果找不到则返回默认位置
    }
};

class BoardComponent : public GUIComponent {
private:
    std::vector<std::unique_ptr<PieceComponent>> pieces;
    PieceComponent* selectedPiece = nullptr;
    sf::Clock animationClock;
    static constexpr float HEX_SIZE = 30.f;
    static constexpr float HEX_HEIGHT = HEX_SIZE * 2.f;
    static constexpr float HEX_WIDTH = HEX_SIZE * sqrt(3.f);

    Board& board;
    std::map<HexCoord, sf::CircleShape> hexagons;
    sf::Vector2f boardCenter;

public:
    BoardComponent(sf::RenderWindow& win, Board& b)
        : GUIComponent(win), board(b) {
        boardCenter = sf::Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f);
        initializeHexGrid();
    }

    void initializeHexGrid() {
        hexagons.clear();
        for (int q = -board.getSize(); q <= board.getSize(); ++q) {
            for (int r = -board.getSize(); r <= board.getSize(); ++r) {
                if (abs(q + r) <= board.getSize()) {
                    HexCoord coord(q, r);
                    sf::Vector2f pos = hexToPixel(coord);

                    sf::CircleShape hex(HEX_SIZE, 6);
                    hex.setOrigin(HEX_SIZE, HEX_SIZE);
                    hex.setPosition(pos);
                    hex.setRotation(30.f);
                    hex.setOutlineThickness(1.f);
                    hex.setOutlineColor(sf::Color::Black);
                    hex.setFillColor(sf::Color(240, 240, 240));

                    hexagons[coord] = hex;
                }
            }
        }
    }

    sf::Vector2f hexToPixel(const HexCoord& hex) {
        float x = boardCenter.x + (HEX_WIDTH * (hex.q + hex.r/2.f));
        float y = boardCenter.y + (HEX_HEIGHT * (3.f/4.f) * hex.r);
        return sf::Vector2f(x, y);
    }

    HexCoord pixelToHex(float x, float y) {
        float q = ((x - boardCenter.x) * 2.f/3.f) / HEX_SIZE;
        float r = ((-x + boardCenter.x) / 3.f + (y - boardCenter.y) * sqrt(3.f)/3.f) / HEX_SIZE;
        return HexCoord(round(q), round(r));
    }

    void createPiece(PieceName type, PlayerID player, const sf::Vector2f& startPos) {
        auto piece = std::make_unique<PieceComponent>(font, type, player);
        piece->setPosition(startPos);
        piece->startAnimation(AnimationType::PLACE,
            startPos + sf::Vector2f(0, -50.f), // 从上方出现
            startPos);
        pieces.push_back(std::move(piece));
    }

    void movePiece(PieceComponent* piece, const HexCoord& newPos) {
        if (!piece) return;

        sf::Vector2f pixelPos = hexToPixel(newPos);
        piece->startAnimation(AnimationType::MOVE,
            piece->getPosition(),
            pixelPos);
        piece->setBoardPosition(newPos);
    }

    void updateAnimations() {
        float deltaTime = animationClock.restart().asSeconds();
        for (auto& piece : pieces) {
            piece->updateAnimation(deltaTime);
        }
    }

    void draw() override {
        // 首先绘制棋盘
        for (auto& [coord, hex] : hexagons) {
            window.draw(hex);

            // 绘制可放置位置的提示（如果有选中的棋子）
            if (selectedPiece && canPlacePieceAt(coord)) {
                sf::CircleShape highlight = hex;
                highlight.setFillColor(sf::Color(100, 255, 100, 50));
                window.draw(highlight);
            }
        }

        // 然后绘制所有棋子
        for (auto& piece : pieces) {
            if (piece.get() != selectedPiece) {
                piece->draw(window);
            }
        }

        // 最后绘制选中的棋子（确保它在最上层）
        if (selectedPiece) {
            selectedPiece->draw(window);
        }
    }

    bool canPlacePieceAt(const HexCoord& coord) {
        if (!selectedPiece) return false;

        // 检查是否是第一个棋子
        if (pieces.empty()) return true;

        // 使用原有游戏逻辑检查位置有效性
        return board.canPlacePiece(coord, selectedPiece->getPlayer());
    }

    void handleEvent(const sf::Event& event) override {
        sf::Vector2f mousePos = window.mapPixelToCoords(
            sf::Vector2i(event.mouseButton.x, event.mouseButton.y));

        if (event.type == sf::Event::MouseButtonPressed) {
            // 尝试选择一个棋子
            for (auto& piece : pieces) {
                if (piece->contains(mousePos)) {
                    selectedPiece = piece.get();
                    selectedPiece->startDrag(mousePos);
                    break;
                }
            }
        }
        else if (event.type == sf::Event::MouseButtonReleased) {
            if (selectedPiece) {
                // 获取释放位置对应的六边形坐标
                HexCoord targetHex = pixelToHex(mousePos.x, mousePos.y);

                // 检查是否可以在该位置放置棋子
                if (canPlacePieceAt(targetHex)) {
                    movePiece(selectedPiece, targetHex);
                } else {
                    // 如果不能放置，返回原始位置
                    if (selectedPiece->isOnBoard()) {
                        movePiece(selectedPiece, selectedPiece->getBoardPosition());
                    } else {
                        // 对于未放置的棋子，返回初始位置
                        selectedPiece->startAnimation(AnimationType::MOVE,
                            selectedPiece->getPosition(),
                            selectedPiece->getInitialPosition(selectedPiece->getType()));
                    }
                }

                selectedPiece->endDrag();
                selectedPiece = nullptr;
            }
        }
        else if (event.type == sf::Event::MouseMoved) {
            if (selectedPiece) {
                selectedPiece->updateDragPosition(mousePos);
            }
        }
    }
};
class MenuSystem;
// 菜单项结构体
struct MenuItem {
    sf::Text text;
    sf::RectangleShape button;
    bool isSelected;

    MenuItem() : isSelected(false) {}
};
class MenuSystem {
private:
    sf::RenderWindow& window;
    sf::Font font;
    std::vector<MenuItem> menuItems;
    sf::Text title;
    sf::Text subtitle;

    MenuState currentState;
    int selectedIndex;

    // 动画相关
    sf::Clock animationClock;
    sf::Clock frameClock; // 新增：用于帧时间计算
    float transitionAlpha;
    bool isTransitioning;
    float buttonHoverScale;

    // 常量
    const float BUTTON_WIDTH = 300.0f;
    const float BUTTON_HEIGHT = 60.0f;
    const float BUTTON_SPACING = 20.0f;
    const float TITLE_Y_POS = 100.0f;
    const float MENU_START_Y = 300.0f;

    bool transitionComplete;  // 新增：标记转换是否完成

    // 私有方法
    void initializeFont() {
        if (!font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
            throw std::runtime_error("Failed to load font");
        }
    }

    void createText(sf::Text& text, const std::string& string,
                   unsigned int size, const sf::Color& color, float y) {
        text.setFont(font);
        text.setString(string);
        text.setCharacterSize(size);
        text.setFillColor(color);

        // 居中文本
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.width / 2, bounds.height / 2);
        text.setPosition(window.getSize().x / 2.0f, y);
    }

    void createMenuItem(const std::string& text, float yPos) {
        MenuItem item;

        // 创建按钮
        item.button.setSize(sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT));
        item.button.setFillColor(sf::Color(50, 50, 50, 200));
        item.button.setOutlineThickness(2.0f);
        item.button.setOutlineColor(sf::Color(100, 100, 100));
        item.button.setOrigin(BUTTON_WIDTH/2, BUTTON_HEIGHT/2);
        item.button.setPosition(window.getSize().x/2, yPos);

        // 创建文本
        createText(item.text, text, 24, sf::Color::White, yPos);

        menuItems.push_back(item);
    }

    void initializeMenu() {
        initializeFont();

        // 创建标题
        createText(title, "Hive Game", 72, sf::Color::Yellow, TITLE_Y_POS);
        createText(subtitle, "A Strategic Board Game", 24,
                  sf::Color(200, 200, 200), TITLE_Y_POS + 80);

        // 创建菜单项
        const std::vector<std::string> options = {
            "Play vs Player",
            "Play vs AI",
            "Settings",
            "Help",
            "Exit"
        };

        for (size_t i = 0; i < options.size(); ++i) {
            createMenuItem(options[i], MENU_START_Y + i * (BUTTON_HEIGHT + BUTTON_SPACING));
        }

        selectedIndex = -1;
        currentState = MenuState::MAIN;
        isTransitioning = false;
        transitionAlpha = 0.0f;
        buttonHoverScale = 1.0f;
    }

public:
    MenuSystem(sf::RenderWindow& win)
        : window(win), selectedIndex(-1), currentState(MenuState::MAIN),
          isTransitioning(false), transitionAlpha(0.0f), buttonHoverScale(1.0f),
          transitionComplete(false) {
        initializeMenu();
        frameClock.restart();
    }

    void update() {
        float deltaTime = frameClock.restart().asSeconds();
        float time = animationClock.getElapsedTime().asSeconds();

        // 更新按钮悬停效果
        for (size_t i = 0; i < menuItems.size(); ++i) {
            if (static_cast<int>(i) == selectedIndex) {
                buttonHoverScale = 1.0f + 0.1f * std::sin(time * 4.0f);
                menuItems[i].button.setScale(buttonHoverScale, buttonHoverScale);
                menuItems[i].text.setScale(buttonHoverScale, buttonHoverScale);
            } else {
                menuItems[i].button.setScale(1.0f, 1.0f);
                menuItems[i].text.setScale(1.0f, 1.0f);
            }
        }

        // 更新过渡效果
        if (isTransitioning) {
            transitionAlpha += 255.0f * deltaTime;
            if (transitionAlpha >= 255.0f) {
                transitionAlpha = 255.0f;
                isTransitioning = false;
                transitionComplete = true;  // 标记转换完成
            }
        }
    }


    void draw() {
        // 绘制背景
        sf::RectangleShape background(sf::Vector2f(
            static_cast<float>(window.getSize().x),
            static_cast<float>(window.getSize().y)));
        background.setFillColor(sf::Color(30, 30, 30));
        window.draw(background);

        // 绘制标题和副标题
        window.draw(title);
        window.draw(subtitle);

        // 绘制菜单项
        for (const auto& item : menuItems) {
            window.draw(item.button);
            window.draw(item.text);
        }

        // 绘制过渡效果
        if (isTransitioning) {
            sf::RectangleShape fadeRect(sf::Vector2f(
                static_cast<float>(window.getSize().x),
                static_cast<float>(window.getSize().y)));
            fadeRect.setFillColor(sf::Color(0, 0, 0,
                static_cast<sf::Uint8>(transitionAlpha)));
            window.draw(fadeRect);
        }
    }

    MenuOption handleInput(const sf::Event& event) {
        if (event.type == sf::Event::MouseMoved) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            selectedIndex = -1;

            for (size_t i = 0; i < menuItems.size(); ++i) {
                if (menuItems[i].button.getGlobalBounds().contains(
                    static_cast<float>(mousePos.x),
                    static_cast<float>(mousePos.y))) {
                    selectedIndex = static_cast<int>(i);
                    break;
                }
            }
        }

        if (event.type == sf::Event::MouseButtonPressed &&
            event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);

            for (size_t i = 0; i < menuItems.size(); ++i) {
                if (menuItems[i].button.getGlobalBounds().contains(
                    static_cast<float>(mousePos.x),
                    static_cast<float>(mousePos.y))) {
                    startTransition();
                    return static_cast<MenuOption>(i);
                }
            }
        }

        return MenuOption::NONE;
    }

    void startTransition() {
        if (!isTransitioning && !transitionComplete) {
            isTransitioning = true;
            transitionAlpha = 0.0f;
        }
    }

    bool isTransitionComplete() const {
        return transitionComplete;
    }

    void resetTransition() {
        transitionComplete = false;
        isTransitioning = false;
        transitionAlpha = 0.0f;
    }

    MenuState getCurrentState() const {
        return currentState;
    }
};
class HiveGUI {
private:
    sf::RenderWindow window;
    GameState state;
    Board board;
    std::unique_ptr<MenuSystem> menuSystem;
    std::unique_ptr<BoardComponent> boardComponent;
    std::unique_ptr<GamePanelComponent> panel;
    MenuOption pendingOption;  // 新增：存储待处理的菜单选项
public:
    HiveGUI()
        : window(sf::VideoMode(1024, 768), "Hive Game", sf::Style::Close),
          state(GameState::MENU),
          board(2),
          pendingOption(MenuOption::NONE) {
        window.setFramerateLimit(60);

        menuSystem = std::make_unique<MenuSystem>(window);
        boardComponent = std::make_unique<BoardComponent>(window, board);
        panel = std::make_unique<GamePanelComponent>(window, board);
    }


    void run() {
        while (window.isOpen()) {
            handleEvents();
            update();
            render();
        }
    }
private:

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            }

            switch (state) {
                case GameState::MENU: {
                    MenuOption option = menuSystem->handleInput(event);
                    if (option != MenuOption::NONE) {
                        pendingOption = option;  // 存储选项，等待过渡完成
                        menuSystem->startTransition();
                    }
                    break;
                }
                case GameState::PLAYING:
                    boardComponent->handleEvent(event);
                break;
                default:
                    break;
            }
        }
    }

    void handleMenuOption(MenuOption option) {
        switch (option) {
            case MenuOption::PLAY_VS_PLAYER:
            case MenuOption::PLAY_VS_AI:
                state = GameState::PLAYING;
            break;
            case MenuOption::SETTINGS:
                // 处理设置选项
                    break;
            case MenuOption::HELP:
                // 显示帮助信息
                    break;
            case MenuOption::EXIT:
                window.close();
            break;
            default:
                break;
        }
    }


    void update() {
        switch (state) {
            case GameState::MENU: {
                menuSystem->update();

                // 检查过渡是否完成以及是否有待处理的选项
                if (menuSystem->isTransitionComplete() && pendingOption != MenuOption::NONE) {
                    handleMenuOption(pendingOption);
                    pendingOption = MenuOption::NONE;  // 重置待处理选项
                    menuSystem->resetTransition();     // 重置过渡状态
                }
                break;
            }
            case GameState::PLAYING:
                // 游戏状态更新逻辑
                    break;
        }
    }


    void render() {
        window.clear(sf::Color(30, 30, 30));  // 深灰色背景

        switch (state) {
            case GameState::MENU:
                menuSystem->draw();
            break;
            case GameState::PLAYING:
                boardComponent->draw();
            panel->draw();
            break;
        }

        window.display();
    }
};

} // namespace piecetype

#endif //HIVE_GUI_HPP
