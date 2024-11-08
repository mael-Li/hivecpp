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
    class PieceComponent;
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
    struct PieceInventory {
        std::vector<std::shared_ptr<PieceComponent>> availablePieces;
        bool hasPlacedQueen;
        int turnCount;

        PieceInventory() : hasPlacedQueen(false), turnCount(0) {}
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
class AnimationSystem {
private:
    struct Animation {
        PieceComponent* piece;
        sf::Vector2f startPos;
        sf::Vector2f endPos;
        float duration;
        float elapsed;
        AnimationType type;
        bool finished;

        Animation(PieceComponent* p, const sf::Vector2f& start,
                 const sf::Vector2f& end, AnimationType t)
            : piece(p), startPos(start), endPos(end),
              duration(0.3f), elapsed(0.0f), type(t), finished(false) {}
    };

    std::vector<Animation> activeAnimations;

public:
    void createMoveAnimation(PieceComponent* piece,
                           const sf::Vector2f& start,
                           const sf::Vector2f& end) {
        activeAnimations.emplace_back(piece, start, end, AnimationType::MOVE);
    }

    void createPlaceAnimation(PieceComponent* piece,
                            const sf::Vector2f& start,
                            const sf::Vector2f& end) {
        activeAnimations.emplace_back(piece, start, end, AnimationType::PLACE);
    }

    void update(float deltaTime) {
        for (auto& anim : activeAnimations) {
            anim.elapsed += deltaTime;
            float progress = std::min(anim.elapsed / anim.duration, 1.0f);

            // 使用缓动函数使动画更平滑
            float easedProgress = easeInOutCubic(progress);

            // 计算当前位置
            sf::Vector2f currentPos;
            switch (anim.type) {
                case AnimationType::MOVE:
                    currentPos = interpolateWithArc(
                        anim.startPos, anim.endPos, easedProgress);
                    break;
                case AnimationType::PLACE:
                    currentPos = interpolateWithBounce(
                        anim.startPos, anim.endPos, easedProgress);
                    break;
                default:
                    currentPos = anim.startPos + (anim.endPos - anim.startPos) * easedProgress;
            }

            // 更新棋子位置
            anim.piece->setPosition(currentPos);

            // 标记完成的动画
            if (progress >= 1.0f) {
                anim.finished = true;
            }
        }

        // 移除完成的动画
        activeAnimations.erase(
            std::remove_if(activeAnimations.begin(), activeAnimations.end(),
                [](const Animation& a) { return a.finished; }),
            activeAnimations.end());
    }

private:
    float easeInOutCubic(float t) {
        return t < 0.5f ? 4 * t * t * t : 1 - pow(-2 * t + 2, 3) / 2;
    }

    sf::Vector2f interpolateWithArc(const sf::Vector2f& start,
                                  const sf::Vector2f& end,
                                  float t) {
        sf::Vector2f direct = start + (end - start) * t;
        float height = 30.0f * std::sin(t * M_PI); // 最大高度30像素
        return direct - sf::Vector2f(0.f, height);
    }

    sf::Vector2f interpolateWithBounce(const sf::Vector2f& start,
                                     const sf::Vector2f& end,
                                     float t) {
        sf::Vector2f direct = start + (end - start) * t;

        // 添加弹跳效果
        if (t > 0.8f) { // 在最后20%的时间添加弹跳
            float bounceProgress = (t - 0.8f) / 0.2f;
            float bounce = 10.0f * std::sin(bounceProgress * M_PI * 2);
            return direct - sf::Vector2f(0.f, bounce);
        }

        return direct;
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
    std::vector<std::unique_ptr<PieceComponent>> boardPieces;
    std::map<HexCoord, sf::CircleShape> hexagons;
    sf::Vector2f boardCenter;

    AnimationSystem animations;

    // 高亮显示
    struct HighlightInfo {
        std::vector<HexCoord> validMoves;
        sf::Color color;
        float alpha;
    } highlight;

public:
    BoardComponent(sf::RenderWindow& win, Board& b)
         : GUIComponent(win), board(b), selectedPiece(nullptr) {
        boardCenter = sf::Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f);
        highlight.color = sf::Color(100, 255, 100);
        highlight.alpha = 0.5f;
        initializeGrid();
    }
    bool handlePieceSelection(const sf::Vector2f& mousePos, PlayerID currentPlayer) {
        HexCoord hexCoord = pixelToHex(mousePos);
        auto piece = board.getTopPiece(hexCoord);

        if (piece && piece->getID() == currentPlayer) {
            // 获取有效移动位置
            highlight.validMoves = board.getPossibleMoves(hexCoord);

            // 如果有有效移动，选中该棋子
            if (!highlight.validMoves.empty()) {
                selectedPiece = getPieceAt(hexCoord);
                return true;
            }
        }
        return false;
    }
    bool handlePieceRelease(const sf::Vector2f& mousePos, PlayerID currentPlayer) {
        if (!selectedPiece) return false;

        HexCoord targetHex = pixelToHex(mousePos);
        bool validMove = false;

        // 检查是否是有效的移动位置
        for (const auto& move : highlight.validMoves) {
            if (move == targetHex) {
                validMove = true;
                break;
            }
        }

        if (validMove) {
            // 执行移动
            HexCoord oldPos = hexFromPiece(selectedPiece);
            try {
                // 创建移动动画
                animations.createMoveAnimation(selectedPiece,
                    hexToPixel(oldPos),
                    hexToPixel(targetHex));

                // 更新棋盘状态
                board.movePiece(oldPos, targetHex);
                updatePiecePositions();

                // 清除选中状态和高亮
                selectedPiece = nullptr;
                highlight.validMoves.clear();

                return true;
            } catch (const InvalidMoveException& e) {
                // 移动失败，返回原位置
                animations.createMoveAnimation(selectedPiece,
                    selectedPiece->getPosition(),
                    hexToPixel(oldPos));
                showError(e.what());
            }
        }

        // 清除选中状态和高亮
        selectedPiece = nullptr;
        highlight.validMoves.clear();
        return false;
    }
    bool canPlacePiece(const HexCoord& coord, PlayerID playerId) const {
        // 第一个棋子可以放在任何位置
        if (board.getTotalPieces() == 0) {
            return true;
        }

        // 检查位置是否已被占用
        if (board.isPositionOccupied(coord)) {
            return false;
        }

        // 检查是否与己方棋子相邻
        bool hasOwnNeighbor = false;
        bool hasEnemyNeighbor = false;

        auto neighbors = coord.neighbors();
        for (const auto& neighbor : neighbors) {
            auto piece = board.getTopPiece(neighbor);
            if (piece) {
                if (piece->getID() == playerId) {
                    hasOwnNeighbor = true;
                } else {
                    hasEnemyNeighbor = true;
                }
            }
        }

        // 必须与己方棋子相邻，且不能与敌方棋子相邻
        return hasOwnNeighbor && !hasEnemyNeighbor;
    }
    bool placePiece(std::shared_ptr<PieceComponent> piece, const HexCoord& coord) {
        if (!canPlacePiece(coord, piece->getPlayer())) {
            return false;
        }

        // 创建放置动画
        animations.createPlaceAnimation(piece.get(),
            piece->getPosition(),
            hexToPixel(coord));

        // 更新棋盘状态
        boardPieces.push_back(std::move(piece));
        board.addPiece(boardPieces.back()->getPiece(), coord,
                      boardPieces.back()->getPlayer());

        return true;
    }


    std::map<HexCoord, sf::CircleShape> hexGrid;
    void initializeGrid() {
        hexGrid.clear();
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

                    hexGrid[coord] = hex;
                }
            }
        }
    }



    sf::Vector2f hexToPixel(const HexCoord& hex) const {
        float x = boardCenter.x + (HEX_WIDTH * (hex.q + hex.r/2.f));
        float y = boardCenter.y + (HEX_HEIGHT * (3.f/4.f) * hex.r);
        return sf::Vector2f(x, y);
    }

    HexCoord pixelToHex(const sf::Vector2f& pixel) const {
        float x = pixel.x - boardCenter.x;
        float y = pixel.y - boardCenter.y;

        float q = (2.f/3.f * x) / HEX_SIZE;
        float r = (-x/3.f + sqrt(3.f)/3.f * y) / HEX_SIZE;
        return HexCoord(round(q), round(r));
    }
    PieceComponent* getPieceAt(const HexCoord& coord) {
        for (auto& piece : boardPieces) {
            if (piece->getBoardPosition() == coord) {
                return piece.get();
            }
        }
        return nullptr;
    }
    HexCoord hexFromPiece(const PieceComponent* piece) {
        for (auto& p : boardPieces) {
            if (p.get() == piece) {
                return p->getBoardPosition();
            }
        }
        return HexCoord(0, 0);
    }
    void showError(const std::string& message) {
        // TODO: Implement error message display
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
        // 绘制棋盘格子
        for (const auto& [coord, hex] : hexGrid) {
            // 绘制基础六边形
            window.draw(hex);

            // 绘制高亮（如果是有效移动位置）
            if (std::find(highlight.validMoves.begin(),
                         highlight.validMoves.end(),
                         coord) != highlight.validMoves.end()) {
                sf::CircleShape highlightHex = hex;
                highlightHex.setFillColor(sf::Color(
                    highlight.color.r,
                    highlight.color.g,
                    highlight.color.b,
                    static_cast<sf::Uint8>(255 * highlight.alpha)
                ));
                window.draw(highlightHex);
                         }
        }

        // 绘制棋子
        for (const auto& piece : boardPieces) {
            if (piece.get() != selectedPiece) {
                piece->draw(window);
            }
        }

        // 最后绘制选中的棋子（确保在最上层）
        if (selectedPiece) {
            selectedPiece->draw(window);
        }

        // 绘制动画
        animations.draw(window);
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

                HexCoord targetHex = pixelToHex(mousePos);

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
    void update(float deltaTime) {
        animations.update(deltaTime);
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

    // AI 系统
class AISystem {
private:
    struct Move {
        HexCoord from;
        HexCoord to;
        int score;
        PieceName pieceType;  // 用于放置新棋子时

        Move(HexCoord f, HexCoord t, int s)
            : from(f), to(t), score(s) {}

        Move(PieceName type, HexCoord t, int s)
            : to(t), score(s), pieceType(type) {}
    };

    Board& board;
    const int MAX_DEPTH = 3;

public:
    AISystem(Board& b) : board(b) {}

    Move getBestMove(PlayerID aiPlayer) {
        std::vector<Move> possibleMoves;

        // 如果需要放置蜂后，优先处理
        if (board.getTurnCount() >= 6 && !board.isQueenPlaced(aiPlayer)) {
            return getBestQueenPlacement(aiPlayer);
        }

        // 收集所有可能的移动
        if (board.isQueenPlaced(aiPlayer)) {
            collectPossibleMoves(aiPlayer, possibleMoves);
        }

        // 如果没有可能的移动，尝试放置新棋子
        if (possibleMoves.empty()) {
            return getBestPiecePlacement(aiPlayer);
        }

        // 评估每个移动的分数
        Move bestMove = possibleMoves[0];
        int bestScore = INT_MIN;

        for (const auto& move : possibleMoves) {
            // 临时执行移动
            auto piece = board.removePiece(move.from);
            board.addPiece(piece, move.to, aiPlayer);

            // 评估局面
            int score = evaluatePosition(aiPlayer);

            // 撤销移动
            board.removePiece(move.to);
            board.addPiece(piece, move.from, aiPlayer);

            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
        }

        return bestMove;
    }

private:
    Move getBestQueenPlacement(PlayerID aiPlayer) {
        std::vector<HexCoord> validPositions;
        int bestScore = INT_MIN;
        HexCoord bestPos(0, 0);

        // 收集所有有效的放置位置
        for (int q = -board.getSize(); q <= board.getSize(); ++q) {
            for (int r = -board.getSize(); r <= board.getSize(); ++r) {
                HexCoord pos(q, r);
                if (board.canPlacePiece(pos, aiPlayer)) {
                    int score = evaluateQueenPosition(pos, aiPlayer);
                    if (score > bestScore) {
                        bestScore = score;
                        bestPos = pos;
                    }
                }
            }
        }

        return Move(PieceName::Queen, bestPos, bestScore);
    }

    Move getBestPiecePlacement(PlayerID aiPlayer) {
        std::vector<std::pair<PieceName, int>> availablePieces;

        // 检查每种棋子的可用数量
        for (const auto& [type, count] : board.piecesAvailable[aiPlayer]) {
            if (count > 0) {
                availablePieces.emplace_back(type, count);
            }
        }

        int bestScore = INT_MIN;
        Move bestMove(PieceName::Queen, HexCoord(0, 0), 0);  // 默认值

        for (const auto& [pieceType, _] : availablePieces) {
            for (int q = -board.getSize(); q <= board.getSize(); ++q) {
                for (int r = -board.getSize(); r <= board.getSize(); ++r) {
                    HexCoord pos(q, r);
                    if (board.canPlacePiece(pos, aiPlayer)) {
                        int score = evaluatePiecePlacement(pieceType, pos, aiPlayer);
                        if (score > bestScore) {
                            bestScore = score;
                            bestMove = Move(pieceType, pos, score);
                        }
                    }
                }
            }
        }

        return bestMove;
    }

    void collectPossibleMoves(PlayerID aiPlayer, std::vector<Move>& moves) {
        for (int q = -board.getSize(); q <= board.getSize(); ++q) {
            for (int r = -board.getSize(); r <= board.getSize(); ++r) {
                HexCoord pos(q, r);
                auto piece = board.getTopPiece(pos);
                if (piece && piece->getID() == aiPlayer) {
                    auto validMoves = board.getPossibleMoves(pos);
                    for (const auto& newPos : validMoves) {
                        moves.emplace_back(pos, newPos, 0);
                    }
                }
            }
        }
    }

    int evaluatePosition(PlayerID aiPlayer) {
        int score = 0;

        // 获取蜂后位置
        PlayerID opponent = (aiPlayer == PlayerID::player1) ?
                            PlayerID::player2 : PlayerID::player1;

        auto queenPositions = board.getqueenBeePositions();
        auto ownQueenIt = queenPositions.find(aiPlayer);
        auto enemyQueenIt = queenPositions.find(opponent);

        // 评估己方蜂后安全性
        if (ownQueenIt != queenPositions.end()) {
            score += evaluateQueenSafety(ownQueenIt->second, aiPlayer);
        }

        // 评估对方蜂后威胁
        if (enemyQueenIt != queenPositions.end()) {
            score -= evaluateQueenSafety(enemyQueenIt->second, opponent);
            score += evaluateQueenThreat(enemyQueenIt->second, aiPlayer);
        }

        // 评估棋子位置和控制
        score += evaluatePieceControl(aiPlayer);

        return score;
    }

    int evaluateQueenSafety(const HexCoord& queenPos, PlayerID player) {
        int safety = 0;
        auto neighbors = queenPos.neighbors();

        // 检查周围的空位
        int emptySpaces = 0;
        int friendlyPieces = 0;
        int enemyPieces = 0;

        for (const auto& neighbor : neighbors) {
            if (!board.isPositionOccupied(neighbor)) {
                emptySpaces++;
            } else {
                auto piece = board.getTopPiece(neighbor);
                if (piece->getID() == player) {
                    friendlyPieces++;
                } else {
                    enemyPieces++;
                }
            }
        }

        // 空位较多意味着更安全
        safety += emptySpaces * 10;
        // 友方棋子提供保护
        safety += friendlyPieces * 15;
        // 敌方棋子造成威胁
        safety -= enemyPieces * 20;

        return safety;
    }

    int evaluateQueenThreat(const HexCoord& enemyQueenPos, PlayerID player) {
        int threat = 0;
        auto neighbors = enemyQueenPos.neighbors();

        // 计算我方棋子对敌方蜂后的包围程度
        int surroundCount = 0;
        for (const auto& neighbor : neighbors) {
            auto piece = board.getTopPiece(neighbor);
            if (piece && piece->getID() == player) {
                surroundCount++;
                threat += 30;  // 每个包围位置都很有价值
            }
        }

        // 如果快要包围成功，大幅提高分数
        if (surroundCount >= 4) {
            threat *= 2;
        }

        return threat;
    }

    int evaluatePieceControl(PlayerID player) {
        int control = 0;

        // 遍历棋盘
        for (int q = -board.getSize(); q <= board.getSize(); ++q) {
            for (int r = -board.getSize(); r <= board.getSize(); ++r) {
                HexCoord pos(q, r);
                auto piece = board.getTopPiece(pos);
                if (piece && piece->getID() == player) {
                    // 根据棋子类型和位置评估控制力
                    switch (piece->getEumName()) {
                        case PieceName::Ant:
                            control += evaluateAntControl(pos);
                            break;
                        case PieceName::Beetle:
                            control += evaluateBeetleControl(pos);
                            break;
                        case PieceName::Grasshopper:
                            control += evaluateGrasshopperControl(pos);
                            break;
                        case PieceName::Spider:
                            control += evaluateSpiderControl(pos);
                            break;
                    }
                }
            }
        }

        return control;
    }

    int evaluateAntControl(const HexCoord& pos) {
        // 蚂蚁控制力基于可移动位置数量
        auto moves = board.getPossibleMoves(pos);
        return moves.size() * 5;
    }

    int evaluateBeetleControl(const HexCoord& pos) {
        // 甲虫控制力基于高度和位置
        int height = board.getStackHeight(pos);
        return height * 15 + 10;
    }

    int evaluateGrasshopperControl(const HexCoord& pos) {
        // 蚱蜢控制力基于可跳跃距离
        auto moves = board.getPossibleMoves(pos);
        int totalDistance = 0;
        for (const auto& move : moves) {
            totalDistance += pos.distance(move);
        }
        return totalDistance * 3;
    }

    int evaluateSpiderControl(const HexCoord& pos) {
        // 蜘蛛控制力基于可达到的关键位置
        auto moves = board.getPossibleMoves(pos);
        int control = moves.size() * 4;

        // 额外检查是否能接近敌方蜂后
        PlayerID opponent = (board.getTopPiece(pos)->getID() == PlayerID::player1) ?
                            PlayerID::player2 : PlayerID::player1;
        auto queenPos = board.getqueenBeePositions().find(opponent);
        if (queenPos != board.getqueenBeePositions().end()) {
            for (const auto& move : moves) {
                if (move.distance(queenPos->second) <= 2) {
                    control += 10;
                }
            }
        }

        return control;
    }
};
// 游戏状态管理系统
class GameStateManager {
private:
    // GameStateManager 实现
    struct PieceInventory {
        std::vector<std::shared_ptr<PieceComponent>> availablePieces;
        bool hasPlacedQueen;
        int turnCount;

        PieceInventory() : hasPlacedQueen(false), turnCount(0) {}
    }; //
    struct GameState {
        PlayerID currentPlayer;
        Board boardState;
        std::map<PlayerID, PieceInventory> inventories;
        bool isGameOver;
        Victory victoryState;

        GameState(const Board& board,
                 const std::map<PlayerID, PieceInventory>& inv,
                 PlayerID current)
            : currentPlayer(current),
              boardState(board),
              inventories(inv),
              isGameOver(false),
              victoryState(Victory::NONE) {}
    };

    std::vector<GameState> stateHistory;
    GameState* currentState;

public:
    GameStateManager(const Board& initialBoard,
                    const std::map<PlayerID, PieceInventory>& initialInventories,
                    PlayerID startingPlayer) {
        stateHistory.emplace_back(initialBoard, initialInventories, startingPlayer);
        currentState = &stateHistory.back();
    }

    void pushState() {
        stateHistory.push_back(*currentState);
        currentState = &stateHistory.back();
    }

    bool undoLastMove() {
        if (stateHistory.size() > 1) {
            stateHistory.pop_back();
            currentState = &stateHistory.back();
            return true;
        }
        return false;
    }

    void updateState(const Board& newBoard,
                    const std::map<PlayerID, PieceInventory>& newInventories,
                    PlayerID newCurrentPlayer) {
        pushState();
        currentState->boardState = newBoard;
        currentState->inventories = newInventories;
        currentState->currentPlayer = newCurrentPlayer;

        // 检查游戏结束条件
        checkGameOver();
    }

    bool isGameOver() const {
        return currentState->isGameOver;
    }

    Victory getVictoryState() const {
        return currentState->victoryState;
    }

private:
    void checkGameOver() {
        // 检查胜利条件
        Victory victory = currentState->boardState.checkVictory();
        if (victory != Victory::NONE) {
            currentState->isGameOver = true;
            currentState->victoryState = victory;
        }
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

    // 新增游戏逻辑相关成员
    PlayerID currentPlayer;
    bool isAIGame;
    std::map<PlayerID, PieceInventory> playerInventories;
    bool gameOver;
    Victory victoryState;

    friend class  HiveGame;
public:
    HiveGUI()
    : window(sf::VideoMode(1024, 768), "Hive Game", sf::Style::Close),
      state(GameState::MENU),
      board(2),
      pendingOption(MenuOption::NONE),
      currentPlayer(PlayerID::player1),
      isAIGame(false),
      gameOver(false),
      victoryState(Victory::NONE) {

        window.setFramerateLimit(60);
        initializeGame();
    }
    void run() {
        while (window.isOpen()) {
            handleEvents();
            update();
            render();
        }
    }
    void handleGameMouseMove(const sf::Event& event) {
        sf::Vector2f mousePos = window.mapPixelToCoords(
            sf::Vector2i(event.mouseMove.x, event.mouseMove.y));

        // 更新拖拽中的棋子位置
        if (auto selectedPiece = boardComponent->getSelectedPiece()) {
            selectedPiece->updateDragPosition(mousePos);
        }
    }
    PlayerID getCurrentPlayer() const {
        return currentPlayer;
    }
    int getTurnCount() const {
        return board.getTurnCount();
    } // 新增: 获取回合数
    const std::map<PlayerID, PieceInventory>& getInventories() const {
        return playerInventories;
    } // 新增: 获取所有玩家的库存
    void HiveGUI::executeMove(const AIMove& move) {
        // AI移动执行逻辑
        if (move.isPlacement) {
            // 处理放置新棋子
            auto piece = std::make_shared<PieceComponent>(
                font, move.pieceType, PlayerID::player2);
            boardComponent->placePiece(piece, move.targetPos);
        } else {
            // 处理移动现有棋子
            boardComponent->movePiece(move.sourcePos, move.targetPos);
        }
    }//新增: 执行AI的移动
private:
    //游戏内容
    void initializeGame() {
        menuSystem = std::make_unique<MenuSystem>(window);
        boardComponent = std::make_unique<BoardComponent>(window, board);
        panel = std::make_unique<GamePanelComponent>(window, board);

        // 初始化玩家库存
        initializePlayerInventory(PlayerID::player1);
        initializePlayerInventory(PlayerID::player2);
    }
    void initializePlayerInventory(PlayerID playerId) {
        PieceInventory& inventory = playerInventories[playerId];

        // 创建初始棋子
        createInitialPieces(inventory, playerId);
    }
    void createInitialPieces(PieceInventory& inventory, PlayerID playerId) {
        // 创建一个蜂后
        inventory.availablePieces.push_back(
            std::make_shared<PieceComponent>(font, PieceName::Queen, playerId));

        // 创建3只蚂蚁
        for(int i = 0; i < 3; ++i) {
            inventory.availablePieces.push_back(
                std::make_shared<PieceComponent>(font, PieceName::Ant, playerId));
        }

        // 创建2只甲虫
        for(int i = 0; i < 2; ++i) {
            inventory.availablePieces.push_back(
                std::make_shared<PieceComponent>(font, PieceName::Beetle, playerId));
        }

        // 创建2只蜘蛛
        for(int i = 0; i < 2; ++i) {
            inventory.availablePieces.push_back(
                std::make_shared<PieceComponent>(font, PieceName::Spider, playerId));
        }

        // 创建3只蚱蜢
        for(int i = 0; i < 3; ++i) {
            inventory.availablePieces.push_back(
                std::make_shared<PieceComponent>(font, PieceName::Grasshopper, playerId));
        }

        // 设置每个棋子的初始位置
        float startX = (playerId == PlayerID::player1) ? 50.f : window.getSize().x - 250.f;
        float y = 50.f;
        float spacing = 60.f;

        for(size_t i = 0; i < inventory.availablePieces.size(); ++i) {
            inventory.availablePieces[i]->setPosition(
                sf::Vector2f(startX + (i % 3) * spacing, y + (i / 3) * spacing)
            );
        }
    }
    void handleMenuOption(MenuOption option) {
        switch (option) {
            case MenuOption::PLAY_VS_PLAYER:
                isAIGame = false;
            state = GameState::PLAYING;
            break;
            case MenuOption::PLAY_VS_AI:
                isAIGame = true;
            state = GameState::PLAYING;
            break;
            case MenuOption::EXIT:
                window.close();
            break;
            default:
                break;
        }
    }
    void handleGameEvents(const sf::Event& event) {
        // 如果是AI的回合且是AI游戏，则不处理玩家输入
        if (isAIGame && currentPlayer == PlayerID::player2) {
            return;
        }

        if (event.type == sf::Event::MouseButtonPressed) {
            handleGameMousePress(event);
        }
        else if (event.type == sf::Event::MouseButtonReleased) {
            handleGameMouseRelease(event);
        }
        else if (event.type == sf::Event::MouseMoved) {
            handleGameMouseMove(event);
        }
    }

    void handleGameMousePress(const sf::Event& event) {
        sf::Vector2f mousePos = window.mapPixelToCoords(
            sf::Vector2i(event.mouseButton.x, event.mouseButton.y));

        // 检查是否点击了玩家的可用棋子
        PieceInventory& currentInventory = playerInventories[currentPlayer];
        for (auto& piece : currentInventory.availablePieces) {
            if (piece->contains(mousePos)) {
                // 检查是否需要强制放置蜂后
                if (currentInventory.turnCount >= 3 && !currentInventory.hasPlacedQueen) {
                    if (piece->getType() != PieceName::Queen) {
                        showMessage("You must place your Queen Bee now!");
                        return;
                    }
                }
                piece->startDrag(mousePos);
                break;
            }
        }

        // 检查是否点击了棋盘上的棋子（用于移动）
        if (board.isQueenPlaced(currentPlayer)) {
            boardComponent->handlePieceSelection(mousePos, currentPlayer);
        }
    }
    void handleGameMouseRelease(const sf::Event& event) {
        sf::Vector2f mousePos = window.mapPixelToCoords(
            sf::Vector2i(event.mouseButton.x, event.mouseButton.y));

        bool moveMade = boardComponent->handlePieceRelease(mousePos, currentPlayer);

        if (moveMade) {
            // 更新游戏状态
            updateGameState();

            // 如果游戏没有结束，切换玩家
            if (!gameOver) {
                switchPlayer();
            }
        }
    }
    void HiveGUI::handleEvents() {
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
                        pendingOption = option;
                        menuSystem->startTransition();
                    }
                    break;
                }
                case GameState::PLAYING:
                    handleGameEvents(event);
                break;
            }
        }
    }
    void updateGameState() {
        PieceInventory& inventory = playerInventories[currentPlayer];
        inventory.turnCount++;

        // 检查胜利条件
        Victory result = board.checkVictory();
        if (result != Victory::NONE) {
            gameOver = true;
            victoryState = result;
            showVictoryMessage();
        }

        // 更新显示
        panel->update(board.getTurnCount(), currentPlayer,
                     playerInventories[currentPlayer].hasPlacedQueen);
    }
    void switchPlayer() {
        currentPlayer = (currentPlayer == PlayerID::player1) ?
                        PlayerID::player2 : PlayerID::player1;

        // 如果是AI游戏且轮到AI
        if (isAIGame && currentPlayer == PlayerID::player2) {
            performAIMove();
        }
    }
    void performAIMove() {
        // AI移动逻辑将在后面实现
        // TODO: Implement AI move logic
    }
    void showMessage(const std::string& message) {
        // 实现消息显示逻辑
        // TODO: Implement message display
    }
    void showVictoryMessage() {
        std::string message;
        switch (victoryState) {
            case Victory::PLAYER1_WINS:
                message = "Player 1 Wins!";
            break;
            case Victory::PLAYER2_WINS:
                message = "Player 2 Wins!";
            break;
            case Victory::DRAW:
                message = "Game is a Draw!";
            break;
            default:
                break;
        }
        showMessage(message);
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
    class UISystem {
private:
    struct Message {
        std::string text;
        sf::Text uiText;
        float duration;
        float elapsed;
        sf::Color color;
        bool isError;

        Message(const std::string& msg, const sf::Font& font, bool error = false)
            : text(msg), duration(error ? 3.0f : 2.0f), elapsed(0.0f), isError(error) {
            uiText.setFont(font);
            uiText.setString(text);
            uiText.setCharacterSize(20);
            color = error ? sf::Color::Red : sf::Color::White;
            uiText.setFillColor(color);
        }
    };

    sf::RenderWindow& window;
    sf::Font font;
    std::vector<Message> messages;
    sf::RectangleShape messageBackground;

    // 持续显示的状态信息
    struct GameInfo {
        sf::Text currentPlayerText;
        sf::Text turnCountText;
        sf::Text piecesAvailableText;
        sf::Text specialRuleText;
    } gameInfo;

public:
    UISystem(sf::RenderWindow& win) : window(win) {
        if (!font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
            throw std::runtime_error("Failed to load font");
        }

        // 初始化消息背景
        messageBackground.setFillColor(sf::Color(0, 0, 0, 180));

        // 初始化游戏信息显示
        initializeGameInfo();
    }

    void showMessage(const std::string& msg, bool isError = false) {
        messages.emplace_back(msg, font, isError);
        updateMessagePositions();
    }

    void showError(const std::string& error) {
        showMessage(error, true);
    }

    void updateGameInfo(PlayerID currentPlayer, int turnCount,
                       const std::map<PlayerID, PieceInventory>& inventories) {
        // 更新当前玩家信息
        std::string playerText = "Current Player: " +
            (currentPlayer == PlayerID::player1 ? "Player 1" : "Player 2");
        gameInfo.currentPlayerText.setString(playerText);

        // 更新回合数
        gameInfo.turnCountText.setString("Turn: " + std::to_string(turnCount));

        // 更新可用棋子信息
        const auto& currentInventory = inventories.at(currentPlayer);
        std::string piecesText = "Available Pieces:\n";
        for (const auto& [type, count] : currentInventory.pieces) {
            piecesText += getPieceTypeName(type) + ": " + std::to_string(count) + "\n";
        }
        gameInfo.piecesAvailableText.setString(piecesText);

        // 更新特殊规则提示
        if (turnCount >= 3 && !currentInventory.hasPlacedQueen) {
            gameInfo.specialRuleText.setString("Must place Queen Bee!");
            gameInfo.specialRuleText.setFillColor(sf::Color::Red);
        } else {
            gameInfo.specialRuleText.setString("");
        }
    }

    void update(float deltaTime) {
        // 更新消息的存在时间和淡出效果
        for (auto it = messages.begin(); it != messages.end();) {
            it->elapsed += deltaTime;

            // 计算透明度
            float alpha = 1.0f - (it->elapsed / it->duration);
            if (alpha <= 0.0f) {
                it = messages.erase(it);
                updateMessagePositions();
            } else {
                sf::Color color = it->color;
                color.a = static_cast<sf::Uint8>(255 * alpha);
                it->uiText.setFillColor(color);
                ++it;
            }
        }
    }

    void draw() {
        // 绘制游戏信息
        window.draw(gameInfo.currentPlayerText);
        window.draw(gameInfo.turnCountText);
        window.draw(gameInfo.piecesAvailableText);
        window.draw(gameInfo.specialRuleText);

        // 绘制消息
        if (!messages.empty()) {
            window.draw(messageBackground);
            for (const auto& msg : messages) {
                window.draw(msg.uiText);
            }
        }
    }

private:
    void initializeGameInfo() {
        // 设置当前玩家文本
        gameInfo.currentPlayerText.setFont(font);
        gameInfo.currentPlayerText.setCharacterSize(20);
        gameInfo.currentPlayerText.setPosition(10, 10);
        gameInfo.currentPlayerText.setFillColor(sf::Color::White);

        // 设置回合数文本
        gameInfo.turnCountText.setFont(font);
        gameInfo.turnCountText.setCharacterSize(20);
        gameInfo.turnCountText.setPosition(10, 40);
        gameInfo.turnCountText.setFillColor(sf::Color::White);

        // 设置可用棋子信息
        gameInfo.piecesAvailableText.setFont(font);
        gameInfo.piecesAvailableText.setCharacterSize(16);
        gameInfo.piecesAvailableText.setPosition(10, 70);
        gameInfo.piecesAvailableText.setFillColor(sf::Color::White);

        // 设置特殊规则提示
        gameInfo.specialRuleText.setFont(font);
        gameInfo.specialRuleText.setCharacterSize(18);
        gameInfo.specialRuleText.setPosition(10, 200);
        gameInfo.specialRuleText.setFillColor(sf::Color::Red);
    }

    void updateMessagePositions() {
        float yPos = window.getSize().y - 150.f;
        float maxWidth = 0.f;

        // 计算消息区域的大小
        for (auto& msg : messages) {
            sf::FloatRect bounds = msg.uiText.getLocalBounds();
            maxWidth = std::max(maxWidth, bounds.width);
            msg.uiText.setPosition(10.f, yPos);
            yPos += 30.f;
        }

        // 更新消息背景的大小和位置
        messageBackground.setSize(sf::Vector2f(maxWidth + 20.f,
                                             messages.size() * 30.f + 10.f));
        messageBackground.setPosition(5.f, window.getSize().y - 155.f);
    }

    std::string getPieceTypeName(PieceName type) {
        switch (type) {
            case PieceName::Queen: return "Queen Bee";
            case PieceName::Ant: return "Ant";
            case PieceName::Beetle: return "Beetle";
            case PieceName::Spider: return "Spider";
            case PieceName::Grasshopper: return "Grasshopper";
            case PieceName::Worm: return "Worm";
            default: return "Unknown";
        }
    }
};
// 最终的游戏集成类
class HiveGame {
private:
    HiveGUI gui;
    AISystem ai;
    GameStateManager stateManager;
    UISystem uiSystem;
    bool isAIGame;
    float aiThinkTime;

public:
    HiveGame()
        : gui(window),
          ai(gui.getBoard()),
          stateManager(gui.getBoard(), gui.getInventories(), PlayerID::player1),
          uiSystem(window),
          isAIGame(false),
          aiThinkTime(0.0f) {

        // 设置初始状态
        updateUIState();
    }

    void run() {
        sf::Clock clock;

        while (window.isOpen()) {
            float deltaTime = clock.restart().asSeconds();

            processEvents();
            update(deltaTime);
            render();
        }
    }

private:
    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::KeyPressed) {
                handleKeyPress(event.key.code);
            } else {
                gui.handleEvent(event);
            }
        }
    }

    void update(float deltaTime) {
        gui.update();
        uiSystem.update(deltaTime);

        // 如果是AI游戏且当前是AI的回合
        if (isAIGame && gui.getCurrentPlayer() == PlayerID::player2 &&
            !stateManager.isGameOver()) {

            aiThinkTime += deltaTime;
            if (aiThinkTime >= 1.0f) {  // 给AI一个思考时间
                performAIMove();
                aiThinkTime = 0.0f;
            }
        }

        // 检查胜利条件
        checkVictoryCondition();
    }

    void render() {
        window.clear(sf::Color(30, 30, 30));
        gui.draw();
        uiSystem.draw();
        window.display();
    }

    void handleKeyPress(sf::Keyboard::Key key) {
        if (key == sf::Keyboard::U) {  // Undo
            if (stateManager.undoLastMove()) {
                updateUIState();
                uiSystem.showMessage("Move undone");
            }
        }
    }

    void performAIMove() {
        auto move = ai.getBestMove(PlayerID::player2);
        gui.executeMove(move);
        updateUIState();
    }

    void checkVictoryCondition() {
        Victory victory = stateManager.getVictoryState();
        if (victory != Victory::NONE) {
            std::string message;
            switch (victory) {
                case Victory::PLAYER1_WINS:
                    message = "Player 1 Wins!";
                    break;
                case Victory::PLAYER2_WINS:
                    message = isAIGame ? "AI Wins!" : "Player 2 Wins!";
                    break;
                case Victory::DRAW:
                    message = "Game is a Draw!";
                    break;
            }
            uiSystem.showMessage(message);
        }
    }

    void updateUIState() {
        uiSystem.updateGameInfo(
            gui.getCurrentPlayer(),
            gui.getTurnCount(),
            gui.getInventories()
        );
    }
};




} // namespace piecetype

#endif //HIVE_GUI_HPP
