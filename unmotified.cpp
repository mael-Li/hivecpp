//
// Created by 李世佳 on 24-11-8.
//
class HiveGUI {
private:
    sf::RenderWindow window;
    GameState state;
    Board board;
    std::unique_ptr<MenuSystem> menuSystem;
    std::unique_ptr<BoardComponent> boardComponent;
    std::unique_ptr<GamePanelComponent> panel;
    MenuOption pendingOption;

    // 新增游戏逻辑相关成员
    PlayerID currentPlayer;
    bool isAIGame;
    std::map<PlayerID, PieceInventory> playerInventories;
    bool gameOver;
    Victory victoryState;

    struct PieceInventory {
        std::vector<std::shared_ptr<PieceComponent>> availablePieces;
        bool hasPlacedQueen;
        int turnCount;

        PieceInventory() : hasPlacedQueen(false), turnCount(0) {}
    };

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

private:
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

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            }

            switch (state) {
                case GameState::MENU:
                    handleMenuEvents(event);
                    break;

                case GameState::PLAYING:
                    if (!gameOver) {
                        handleGameEvents(event);
                    }
                    break;
            }
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
    class BoardComponent : public GUIComponent {
private:
    Board& board;
    std::vector<std::unique_ptr<PieceComponent>> boardPieces;
    PieceComponent* selectedPiece;
    sf::Vector2f boardCenter;

    // 高亮显示
    struct HighlightInfo {
        std::vector<HexCoord> validMoves;
        sf::Color color;
        float alpha;
    } highlight;

    // 动画系统
    AnimationSystem animations;

    static constexpr float HEX_SIZE = 30.f;
    static constexpr float HEX_HEIGHT = HEX_SIZE * 2.f;
    static constexpr float HEX_WIDTH = HEX_SIZE * sqrt(3.f);

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

    void update(float deltaTime) {
        animations.update(deltaTime);
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

private:
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
    // UI消息系统
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
        gui.update(deltaTime);
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
    int main() {
        try {
            HiveGame game;
            game.run();
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
        return 0;
    }