//
// Created by 李世佳 on 24-11-2.
//

#include "Scene_manager.hpp"

#include <iostream>

const std::string Button::FONT_PATH = "resources/fonts/static/NotoSans-Regular.ttf";
bool Button::loadFont() {
    // 直接使用 Windows 系统字体，避免资源路径问题
    if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
        return true;
    }

    std::cerr << "Failed to load font in Button" << std::endl;
    return false;
}
// 修改Button构造函数
Button::Button(const std::string& text, const sf::Vector2f& position, const sf::Vector2f& size)
    : rect(size) {
    rect.setPosition(position);
    rect.setFillColor(sf::Color(100, 100, 100));

    if (!loadFont()) {
        std::cerr << "Warning: Button created without proper font" << std::endl;
        // 继续创建按钮，但可能没有文字显示
    }

    buttonText.setFont(font);
    buttonText.setString(text);
    buttonText.setCharacterSize(24);
    buttonText.setFillColor(sf::Color::White);

    // 居中文本
    sf::FloatRect textBounds = buttonText.getLocalBounds();
    buttonText.setPosition(
        position.x + (size.x - textBounds.width) / 2.0f,
        position.y + (size.y - textBounds.height) / 2.0f
    );
}
bool Button::contains(sf::Vector2f point) const {
    return rect.getGlobalBounds().contains(point);
}

void Button::setHovered(bool hovered) {
    rect.setFillColor(hovered ? sf::Color(150, 150, 150) : sf::Color(100, 100, 100));
}

// 在渲染时添加错误检查
void Button::render(sf::RenderWindow& window) {
    window.draw(rect);
    // 只有当字体加载成功时才绘制文本
    if (font.getInfo().family != "") {
        window.draw(buttonText);
    }
}
void SceneManager::init() {
    std::cout << "Initializing game..." << std::endl;  // 添加调试输出

    // 创建窗口
    window.create(
        sf::VideoMode(800, 600),
        "Hive Game",
        sf::Style::Default
    );

    // 设置帧率限制
    window.setFramerateLimit(60);

    // 初始化第一个场景
    currentScene = std::make_unique<MainMenuScene>();

    std::cout << "Initialization complete" << std::endl;  // 添加调试输出
}
void SceneManager::run() {
    std::cout << "Starting game loop..." << std::endl;  // 添加调试输出

    while (window.isOpen()) {
        // 处理事件
        sf::Event event;
        while (window.pollEvent(event)) {
            // 窗口关闭事件
            if (event.type == sf::Event::Closed) {
                window.close();
                continue;
            }

            // 其他事件传递给当前场景处理
            if (currentScene) {
                currentScene->handleEvent(event);
            }
        }

        // 更新当前场景
        if (currentScene) {
            currentScene->update();
        }

        // 清除窗口
        window.clear(sf::Color(50, 50, 50));  // 深灰色背景

        // 渲染当前场景
        if (currentScene) {
            currentScene->render(window);
        }

        // 显示绘制的内容
        window.display();
    }

    std::cout << "Game loop ended" << std::endl;  // 添加调试输出
}

void SceneManager::setScene(std::unique_ptr<Scene> scene) {
    try {
        if (scene) {
            std::cout << "Changing scene to new scene" << std::endl;
            currentScene = std::move(scene);

            // 立即清除并重绘窗口以显示新场景
            window.clear(sf::Color(50, 50, 50));
            currentScene->render(window);
            window.display();

            std::cout << "Scene changed and rendered successfully" << std::endl;
        } else {
            std::cerr << "Attempted to set null scene" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error changing scene: " << e.what() << std::endl;
        throw;
    }
}

MainMenuScene::MainMenuScene() {
    std::cout << "Creating MainMenuScene..." << std::endl;  // 添加调试输出

    try {
        // 加载字体
        if (!font.loadFromFile("C:\\Windows\\Fonts\\consola.ttf")) {
            throw std::runtime_error("Failed to load font");
        }

        // 创建按钮
        buttons.clear();
        buttons.push_back(std::make_unique<Button>(
            "Start Game",
            sf::Vector2f(300, 200),
            sf::Vector2f(200, 50)
        ));

        buttons.push_back(std::make_unique<Button>(
            "Exit",
            sf::Vector2f(300, 300),
            sf::Vector2f(200, 50)
        ));

        std::cout << "MainMenuScene created successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error in MainMenuScene constructor: " << e.what() << std::endl;
        throw;
    }
}


void MainMenuScene::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);

        for (size_t i = 0; i < buttons.size(); ++i) {
            if (buttons[i]->contains(mousePos)) {
                std::cout << "Button " << i << " clicked" << std::endl;

                switch (i) {
                    case 0: { // Start Game
                        std::cout << "Creating GameModeScene..." << std::endl;
                        auto gameModeScene = std::make_unique<GameModeScene>();
                        SceneManager::getInstance().setScene(std::move(gameModeScene));
                        std::cout << "GameModeScene set" << std::endl;
                        return;  // 添加 return 以防止继续处理
                    }
                    case 1: // Exit
                        SceneManager::getInstance().getWindow().close();
                    return;
                }
            }
        }
    }
    // 更新按钮悬停状态
    else if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mousePos(event.mouseMove.x, event.mouseMove.y);
        for (auto& button : buttons) {
            button->setHovered(button->contains(mousePos));
        }
    }
}

void MainMenuScene::update() {
    // 更新按钮状态
    sf::Vector2i mousePos = sf::Mouse::getPosition(
        SceneManager::getInstance().getWindow()
    );

    for (auto& button : buttons) {
        button->setHovered(button->contains(sf::Vector2f(mousePos)));
    }
}

void MainMenuScene::render(sf::RenderWindow& window) {
    // 清除窗口
    window.clear(sf::Color(50, 50, 50));

    // 绘制所有按钮
    for (const auto& button : buttons) {
        button->render(window);
    }
}
GameModeScene::GameModeScene() {
    std::cout << "GameModeScene constructor start" << std::endl;

    try {
        if (!font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
            throw std::runtime_error("Failed to load font in GameModeScene");
        }
        std::cout << "Font loaded successfully" << std::endl;

        // 创建按钮
        float centerX = 300.0f;  // 可以根据窗口大小调整
        float startY = 200.0f;
        float buttonSpacing = 80.0f;

        buttons.clear();
        buttons.push_back(std::make_unique<Button>(
            "Player vs Player",
            sf::Vector2f(centerX, startY),
            sf::Vector2f(200, 50)
        ));

        buttons.push_back(std::make_unique<Button>(
            "Player vs AI",
            sf::Vector2f(centerX, startY + buttonSpacing),
            sf::Vector2f(200, 50)
        ));

        buttons.push_back(std::make_unique<Button>(
            "Back",
            sf::Vector2f(centerX, startY + 2 * buttonSpacing),
            sf::Vector2f(200, 50)
        ));

        std::cout << "GameModeScene buttons created" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error in GameModeScene constructor: " << e.what() << std::endl;
        throw;
    }
}

void GameModeScene::createButtons() {
    // 清除现有按钮
    buttons.clear();

    // 计算按钮位置（居中）
    float windowWidth = static_cast<float>(SceneManager::getInstance().getWindow().getSize().x);
    float windowHeight = static_cast<float>(SceneManager::getInstance().getWindow().getSize().y);
    float buttonWidth = 200.0f;
    float buttonHeight = 50.0f;
    float buttonSpacing = 20.0f;  // 按钮之间的间距

    sf::Vector2f buttonSize(buttonWidth, buttonHeight);

    // 创建三个按钮，垂直排列在屏幕中央
    float startY = (windowHeight - (3 * buttonHeight + 2 * buttonSpacing)) / 2;
    float centerX = (windowWidth - buttonWidth) / 2;

    buttons.push_back(std::make_unique<Button>(
        "Player vs Player",
        sf::Vector2f(centerX, startY),
        buttonSize
    ));

    buttons.push_back(std::make_unique<Button>(
        "Player vs AI",
        sf::Vector2f(centerX, startY + buttonHeight + buttonSpacing),
        buttonSize
    ));

    buttons.push_back(std::make_unique<Button>(
        "Back",
        sf::Vector2f(centerX, startY + 2 * (buttonHeight + buttonSpacing)),
        buttonSize
    ));
}

void GameModeScene::handleEvent(const sf::Event& event) {
    try {
        if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);

            for (size_t i = 0; i < buttons.size(); ++i) {
                if (buttons[i]->contains(mousePos)) {
                    std::cout << "Button clicked: " << i << std::endl;

                    switch (i) {
                        case 0:  // Player vs Player
                            std::cout << "Starting Player vs Player mode" << std::endl;
                            SceneManager::getInstance().setScene(
                                std::make_unique<GameplayScene>(GameMode::PLAYER_VS_PLAYER)
                            );
                            break;

                        case 1:  // Player vs AI
                            std::cout << "Starting Player vs AI mode" << std::endl;
                            SceneManager::getInstance().setScene(
                                std::make_unique<GameplayScene>(GameMode::PLAYER_VS_AI)
                            );
                            break;

                        case 2:  // Back
                            std::cout << "Returning to main menu" << std::endl;
                            SceneManager::getInstance().setScene(
                                std::make_unique<MainMenuScene>()
                            );
                            break;
                    }
                }
            }
        }
        else if (event.type == sf::Event::MouseMoved) {
            sf::Vector2f mousePos(event.mouseMove.x, event.mouseMove.y);
            for (auto& button : buttons) {
                button->setHovered(button->contains(mousePos));
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in GameModeScene::handleEvent: " << e.what() << std::endl;
    }
}

void GameModeScene::update() {}

void GameModeScene::render(sf::RenderWindow& window) {
    // 清除窗口
    window.clear(sf::Color(50, 50, 50));

    // 绘制标题
    sf::Text title;
    title.setFont(font);
    title.setString("Select Game Mode");
    title.setCharacterSize(32);
    title.setFillColor(sf::Color::White);
    title.setPosition(250, 100);  // 调整位置
    window.draw(title);

    // 绘制所有按钮
    for (const auto& button : buttons) {
        button->render(window);
    }
}

GameplayScene::GameplayScene(GameMode mode) :
    gameMode(mode),
    isPieceSelected(false),
    selectedPiecePos(0, 0),
    game(std::make_unique<piecetype::Game>())  // 直接在初始化列表中创建游戏对象
{
    std::cout << "Creating GameplayScene" << std::endl;
    try {
        initializeGraphics();
        std::cout << "GameplayScene created successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error in GameplayScene constructor: " << e.what() << std::endl;
        throw;
    }
}
// GameplayScene.cpp
void GameplayScene::initializeGraphics() {
    try {
        // 加载字体
        if (!font.loadFromFile("C:\\Windows\\Fonts\\consola.ttf")) {
            throw std::runtime_error("Failed to load font");
        }

        // 初始化玩家颜色
        playerColors[piecetype::PlayerID::player1] = sf::Color(200, 100, 100);  // 红色
        playerColors[piecetype::PlayerID::player2] = sf::Color(100, 100, 200);  // 蓝色
        playerColors[piecetype::PlayerID::playerai] = sf::Color(100, 200, 100); // 绿色

        // 设置状态文本
        statusText.setFont(font);
        statusText.setCharacterSize(20);
        statusText.setFillColor(sf::Color::White);
        statusText.setPosition(10, 10);

        // 初始化选中高亮
        selectedPieceHighlight.setFillColor(sf::Color(255, 255, 0, 100));

        std::cout << "Graphics initialized successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error initializing graphics: " << e.what() << std::endl;
        throw;
    }
}
sf::Vector2f GameplayScene::hexToScreen(const HexCoord& hex) {
    const float sqrt3 = std::sqrt(3.0f);
    float x = BOARD_OFFSET_X + HEX_SIZE * (sqrt3 * hex.q + sqrt3/2.0f * hex.r);
    float y = BOARD_OFFSET_Y + HEX_SIZE * (3.0f/2.0f * hex.r);
    return {x, y};
}
HexCoord GameplayScene::screenToHex(const sf::Vector2f& screen) {
    const float sqrt3 = std::sqrt(3.0f);
    float q = ((screen.x - BOARD_OFFSET_X) * sqrt3/3.0f - (screen.y - BOARD_OFFSET_Y) / 3.0f) / HEX_SIZE;
    float r = (screen.y - BOARD_OFFSET_Y) * 2.0f/3.0f / HEX_SIZE;

    // 使用立方体坐标进行四舍五入
    float s = -q - r;
    int qi = std::round(q);
    int ri = std::round(r);
    int si = std::round(s);

    // 修正立方体坐标
    float q_diff = std::abs(qi - q);
    float r_diff = std::abs(ri - r);
    float s_diff = std::abs(si - s);

    if (q_diff > r_diff && q_diff > s_diff) {
        qi = -ri - si;
    } else if (r_diff > s_diff) {
        ri = -qi - si;
    }

    return HexCoord(qi, ri);
}
void GameplayScene::drawHexagon(sf::RenderWindow& window, const HexCoord& pos, sf::Color color) {
    const float sqrt3 = std::sqrt(3.0f);
    sf::ConvexShape hexagon;
    hexagon.setPointCount(6);

    sf::Vector2f center = hexToScreen(pos);

    for (int i = 0; i < 6; ++i) {
        float angle = i * 60.0f * M_PI / 180.0f;
        float x = center.x + HEX_SIZE * std::cos(angle);
        float y = center.y + HEX_SIZE * std::sin(angle);
        hexagon.setPoint(i, sf::Vector2f(x, y));
    }

    hexagon.setFillColor(sf::Color::Transparent);
    hexagon.setOutlineColor(color);
    hexagon.setOutlineThickness(2.0f);
    window.draw(hexagon);
}

void GameplayScene::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            handleMouseClick({event.mouseButton.x, event.mouseButton.y});
        }
    }
}
void GameplayScene::handleMouseClick(sf::Vector2i mousePos) {
    HexCoord clickedHex = screenToHex(sf::Vector2f(mousePos));

    // 在这里处理游戏逻辑
    // 1. 如果没有选中棋子且点击了有棋子的格子，选中该棋子
    // 2. 如果已经选中棋子且点击了空格子，尝试移动棋子
    // 3. 如果点击了其他位置，取消选中

    // 示例代码：
    if (game->getBoard().isPositionOccupied(clickedHex)) {
        selectedPiecePos = clickedHex;
        isPieceSelected = true;
    } else if (isPieceSelected) {
        // 尝试移动棋子
        try {
            auto piece = game->getBoard().getPieceAt(selectedPiecePos);
            if (piece) {
                piece->move(game->getBoard(), clickedHex, piece->getID());
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid move: " << e.what() << std::endl;
        }
        isPieceSelected = false;
    }
}

void GameplayScene::update() {
    // 更新游戏状态
}

void GameplayScene::render(sf::RenderWindow& window) {
    try {
        window.clear(sf::Color(30, 30, 30));  // 深色背景

        // 绘制棋盘
        drawBoard(window);

        // 绘制UI
        window.draw(statusText);

        // 绘制调试信息
        if (isPieceSelected) {
            std::string debugText = "Selected: " + std::to_string(selectedPiecePos.q) +
                                  ", " + std::to_string(selectedPiecePos.r);
            sf::Text text;
            text.setFont(font);
            text.setString(debugText);
            text.setCharacterSize(16);
            text.setFillColor(sf::Color::White);
            text.setPosition(10, window.getSize().y - 30);
            window.draw(text);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in GameplayScene::render: " << e.what() << std::endl;
    }
}


void GameplayScene::drawBoard(sf::RenderWindow& window) {
    if (!game) {
        std::cerr << "Game object not initialized!" << std::endl;
        return;
    }

    // 绘制背景网格
    const int gridSize = 8;
    for (int q = -gridSize; q <= gridSize; ++q) {
        for (int r = -gridSize; r <= gridSize; ++r) {
            // 确保六边形在合理范围内
            if (std::abs(q + r) <= gridSize) {
                HexCoord hex(q, r);
                // 绘制空格子
                drawHexagon(window, hex, sf::Color(70, 70, 70));
            }
        }
    }

    // 绘制棋子
    const auto& board = game->getBoard();
    for (int q = -gridSize; q <= gridSize; ++q) {
        for (int r = -gridSize; r <= gridSize; ++r) {
            if (std::abs(q + r) <= gridSize) {
                HexCoord hex(q, r);
                if (board.isPositionOccupied(hex)) {
                    auto piece = board.getPieceAt(hex);
                    if (piece) {
                        drawPiece(window, *piece);
                    }
                }
            }
        }
    }

    // 绘制选中高亮
    if (isPieceSelected) {
        sf::Vector2f pos = hexToScreen(selectedPiecePos);
        sf::CircleShape highlight(HEX_SIZE);
        highlight.setPointCount(6);
        highlight.setPosition(pos.x - HEX_SIZE, pos.y - HEX_SIZE);
        highlight.setFillColor(sf::Color(255, 255, 0, 50));
        highlight.setOutlineColor(sf::Color::Yellow);
        highlight.setOutlineThickness(2.0f);
        window.draw(highlight);
    }
}
void GameplayScene::drawPiece(sf::RenderWindow& window, const piecetype::Piece& piece) {
    sf::Vector2f pos = hexToScreen(piece.getPosition());

    // 创建棋子形状
    sf::CircleShape pieceShape(HEX_SIZE * 0.8f);
    pieceShape.setPointCount(6);
    pieceShape.setPosition(
        pos.x - pieceShape.getRadius(),
        pos.y - pieceShape.getRadius()
    );

    // 设置颜色
    sf::Color baseColor = playerColors[piece.getID()];
    pieceShape.setFillColor(baseColor);
    pieceShape.setOutlineColor(sf::Color::White);
    pieceShape.setOutlineThickness(2.0f);

    // 绘制棋子
    window.draw(pieceShape);

    // 绘制标识文本
    sf::Text pieceText;
    pieceText.setFont(font);
    pieceText.setString(piece.getName());
    pieceText.setCharacterSize(static_cast<unsigned int>(HEX_SIZE * 0.6f));
    pieceText.setFillColor(sf::Color::White);

    // 居中文本
    sf::FloatRect textBounds = pieceText.getLocalBounds();
    pieceText.setPosition(
        pos.x - textBounds.width * 0.5f,
        pos.y - textBounds.height * 0.5f
    );

    window.draw(pieceText);
}