//
// Created by 李世佳 on 24-10-17.
//
#include "Game.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include <thread>
#include <chrono>

namespace piecetype {
    // 定义全局常量
    const std::map<std::string, PieceName> PIECE_TYPES = {
        {"Q", PieceName::Queen},
        {"A", PieceName::Ant},
        {"S", PieceName::Spider},
        {"B", PieceName::Beetle},
        {"G", PieceName::Grasshopper},
        {"W", PieceName::Worm}
    };

    const std::map<GameCommand, std::string> COMMAND_DESCRIPTIONS = {
        {GameCommand::PLACE_PIECE, "Place a new piece"},
        {GameCommand::MOVE_PIECE, "Move an existing piece"},
        {GameCommand::SHOW_HELP, "Show help"},
        {GameCommand::SHOW_BOARD, "Show board"},
        {GameCommand::SHOW_STATS, "Show statistics"},
        {GameCommand::QUIT, "Exit"}
    };

    HumanPlayer::HumanPlayer(const std::string& name, const PlayerID& id)
        : Player(name, id) {}

    void HumanPlayer::makeMove(Board& board, GameCommand command) {
        switch (command) {
            case GameCommand::PLACE_PIECE:
                handlePlacePiece(board);
            break;
            case GameCommand::MOVE_PIECE:
                handleMovePiece(board);
            break;
            default:
                throw std::runtime_error("Invalid command for player action");
        }
        incrementMoveCount();
    }

void HumanPlayer::handlePlacePiece(Board& board) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    bool queenRequired = (board.getTurnCount() >= 6 && !board.isQueenPlaced(getID()));

    if (queenRequired) {
        SetConsoleTextAttribute(hConsole, 12); // 红色
        std::cout << "\nYou must place your Queen Bee this turn!\n";
        SetConsoleTextAttribute(hConsole, 7); // 恢复默认色
        placePiece(board, "Q");
        return;
    }

    // 显示可用棋子
    SetConsoleTextAttribute(hConsole, 11); // 亮青色
    std::cout << "\nAvailable pieces to place:\n";
    std::cout << "+--------------------------------+\n";

    for (const auto& [symbol, type] : PIECE_TYPES) {
        int remaining = board.piecesAvailable[getID()][type];
        if (remaining > 0) {
            std::string pieceName;
            std::string description;
            switch (type) {
                case PieceName::Queen:
                    pieceName = "Queen Bee";
                    description = "Moves one space";
                    SetConsoleTextAttribute(hConsole, 14); // 黄色
                    break;
                case PieceName::Ant:
                    pieceName = "Ant";
                    description = "Unlimited movement";
                    SetConsoleTextAttribute(hConsole, 10); // 绿色
                    break;
                case PieceName::Spider:
                    pieceName = "Spider";
                    description = "Moves exactly three spaces";
                    SetConsoleTextAttribute(hConsole, 13); // 紫色
                    break;
                case PieceName::Beetle:
                    pieceName = "Beetle";
                    description = "Can climb on other pieces";
                    SetConsoleTextAttribute(hConsole, 9); // 蓝色
                    break;
                case PieceName::Grasshopper:
                    pieceName = "Grasshopper";
                    description = "Jumps over pieces";
                    SetConsoleTextAttribute(hConsole, 11); // 青色
                    break;
                case PieceName::Worm:
                    pieceName = "Worm";
                    description = "Moves under and swaps";
                    SetConsoleTextAttribute(hConsole, 12); // 红色
                    break;
                default:
                    continue;
            }
            std::cout << "| " << symbol << " - " << std::left << std::setw(12)
                      << pieceName << " | " << remaining << " remaining | "
                      << description << "\n";
        }
    }
    std::cout << "+--------------------------------+\n";

    // 获取玩家选择的棋子类型
    std::string pieceType;
    bool validPieceType = false;
    do {
        SetConsoleTextAttribute(hConsole, 15); // 亮白色
        std::cout << "\nEnter piece type ";
        if (board.piecesAvailable[getID()][PieceName::Worm] > 0) {
            std::cout << "(Q/A/S/B/G/W): ";
        } else {
            std::cout << "(Q/A/S/B/G): ";
        }

        std::cin >> pieceType;
        std::transform(pieceType.begin(), pieceType.end(), pieceType.begin(), ::toupper);

        if (PIECE_TYPES.find(pieceType) != PIECE_TYPES.end()) {
            if (board.piecesAvailable[getID()][PIECE_TYPES.at(pieceType)] > 0) {
                validPieceType = true;
            } else {
                SetConsoleTextAttribute(hConsole, 12); // 红色
                std::cout << "No more " << pieceType << " pieces available.\n";
            }
        } else {
            SetConsoleTextAttribute(hConsole, 12); // 红色
            std::cout << "Invalid piece type. Please try again.\n";
        }
    } while (!validPieceType);

    SetConsoleTextAttribute(hConsole, 7); // 恢复默认色
    // 放置选择的棋子
    placePiece(board, pieceType);
}

void HumanPlayer::placePiece(Board& board, const std::string& pieceType) {
    int maxAttempts = 3; // 最大尝试次数
    int attempts = 0;

    while (attempts < maxAttempts) {
        try {
            int x, y;
            std::cout << "Enter coordinates (x y) to place " << pieceType << ": ";
            if (!(std::cin >> x >> y)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                throw InvalidMoveException("Invalid input format. Please enter two numbers.");
            }

            if (!validateCoordinates(x, y, board)) {
                throw InvalidMoveException("Coordinates out of board bounds.");
            }

            auto piece = createPiece(pieceType);
            if (!piece) {
                throw InvalidMoveException("Failed to create piece.");
            }

            board.addPiece(piece, HexCoord(x, y), getID());
            std::cout << "Successfully placed " << pieceType << " at (" << x << "," << y << ")\n";
            return;

        } catch (const InvalidMoveException& e) {
            attempts++;
            std::cout << "Error: " << e.what() << "\n";
            if (attempts < maxAttempts) {
                std::cout << "Please try again (" << (maxAttempts - attempts)
                         << " attempts remaining).\n";
            }
        } catch (const std::exception& e) {
            attempts++;
            std::cout << "Unexpected error: " << e.what() << "\n";
            if (attempts < maxAttempts) {
                std::cout << "Please try again (" << (maxAttempts - attempts)
                         << " attempts remaining).\n";
            }
        }
    }

    throw InvalidMoveException("Maximum placement attempts reached. Turn skipped.");
}


    void HumanPlayer::handleMovePiece(Board& board) {
        if (!board.isQueenPlaced(getID())) {
            throw InvalidMoveException("You must place your Queen Bee before moving any pieces!");
        }

        int fromX, fromY;
        std::cout << "Enter the coordinates of the piece to move (x y): ";
        if (!(std::cin >> fromX >> fromY)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            throw InvalidMoveException("Invalid input format for source coordinates.");
        }

        if (!validateCoordinates(fromX, fromY, board)) {
            throw InvalidMoveException("Source coordinates out of bounds.");
        }

        auto piece = board.getPieceAt(HexCoord(fromX, fromY));
        if (!piece || piece->getID() != getID()) {
            throw InvalidMoveException("No valid piece at the selected position.");
        }

        int toX, toY;
        std::cout << "Enter the target coordinates (x y): ";
        if (!(std::cin >> toX >> toY)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            throw InvalidMoveException("Invalid input format for target coordinates.");
        }

        if (!validateCoordinates(toX, toY, board)) {
            throw InvalidMoveException("Target coordinates out of bounds.");
        }

        piece->move(board, HexCoord(toX, toY), getID());
        std::cout << "Piece moved successfully from (" << fromX << "," << fromY
                  << ") to (" << toX << "," << toY << ")\n";
    }

    std::shared_ptr<Piece> HumanPlayer::createPiece(const std::string& pieceType) {
        if (PIECE_TYPES.find(pieceType) == PIECE_TYPES.end()) {
            return nullptr;
        }

        switch (PIECE_TYPES.at(pieceType)) {
            case PieceName::Queen: return std::make_shared<QueenBee>(id);
            case PieceName::Ant: return std::make_shared<Ant>(id);
            case PieceName::Spider: return std::make_shared<Spider>(id);
            case PieceName::Beetle: return std::make_shared<Beetle>(id);
            case PieceName::Grasshopper: return std::make_shared<Grasshopper>(id);
            case PieceName::Worm: return std::make_shared<Worm>(id);
            default: return nullptr;
        }
    }

    bool HumanPlayer::validateCoordinates(int x, int y, const Board& board) const {
        if (!board.isValidPosition(HexCoord(x, y))) {
            std::cout << "Invalid coordinates. Please try again.\n";
            return false;
        }
        return true;
    }

    // 在 Game.cpp 中修改构造函数实现
    Game::Game(bool isPvP, bool useExtendedPieces)
        : board(BOARD_SIZE),
          gameState(GameState::MENU),
          hasExtendedPieces(useExtendedPieces),
          isPvP(isPvP) {

        // 初始化游戏
        initializeGame();

        // 如果使用扩展棋子，为每个玩家添加蚯蚓棋子
        if (hasExtendedPieces) {
            board.piecesAvailable[PlayerID::player1][PieceName::Worm] = 1;
            board.piecesAvailable[PlayerID::player2][PieceName::Worm] = 1;
        }
    }

    void Game::initializeGame() {
        // 创建玩家
        players.clear();
        players.push_back(std::make_shared<HumanPlayer>("Player 1", PlayerID::player1));

        // 根据游戏模式创建对应的玩家2
        if (isPvP) {
            players.push_back(std::make_shared<HumanPlayer>("Player 2", PlayerID::player2));
        } else {
            players.push_back(std::make_shared<AIPlayer>("AI Player", PlayerID::player2));
        }

        // 初始化当前玩家
        currentPlayer = players[0];

        // 初始化玩家状态
        playerStates.clear();
        playerStates[PlayerID::player1] = PlayerState();
        playerStates[PlayerID::player2] = PlayerState();

        // 初始化游戏统计
        stats = GameStats();
        stats.movesMade[PlayerID::player1] = 0;
        stats.movesMade[PlayerID::player2] = 0;
        stats.queenPlaced[PlayerID::player1] = false;
        stats.queenPlaced[PlayerID::player2] = false;

        gameState = GameState::PLAYING;
    }


    void Game::start() {
        clearScreen();
        std::cout << "\n=== Welcome to Hive Game ===\n\n";
        displayHelp();
        gameLoop();
    }

    void Game::gameLoop() {
        while (gameState != GameState::GAME_OVER) {
            displayGameStatus();

            GameCommand command;
            if (currentPlayer->isHuman()) {
                displayMenu();
                command = getCommand();
                if (command == GameCommand::QUIT) {
                    break;
                }
            } else {
                // AI玩家自动选择放置棋子或移动棋子
                command = board.isQueenPlaced(currentPlayer->getID()) &&
                         board.getTotalPieces() > 0 ?
                         GameCommand::MOVE_PIECE : GameCommand::PLACE_PIECE;
                // 添加延迟让AI看起来在思考
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }

            try {
                handleGameCommand(command);
                updateGameState();

                if (command == GameCommand::PLACE_PIECE || command == GameCommand::MOVE_PIECE) {
                    switchPlayer();
                }
            }
            catch (const std::exception& e) {
                std::cout << "\nError: " << e.what() << "\n";
                if (currentPlayer->isHuman()) {
                    std::cout << "Press Enter to continue...";
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                }
            }
        }

        if (isGameOver()) {
            std::cout << "\nGame Over! " << getWinner() << " wins!\n";
        }
    }
    void Game::displayStartScreen() {
        // 设置为青色
        setColor(11);
        std::cout << "================================\n";

        // 设置为黄色
        setColor(14);
        std::cout << "           HIVE GAME\n";

        // 设置为青色
        setColor(11);
        std::cout << "================================\n\n";

        // 设置为黄色，显示ASCII艺术
        setColor(14);
        std::cout << "    \\/\n";
        std::cout << "    /\\_/\\\n";
        std::cout << "   (>°.°<)\n";
        std::cout << "    \\_^_/\n\n";

        // 设置为白色，显示游戏描述
        setColor(15);
        std::cout << "A strategic board game where you must\n";
        std::cout << "surround your opponent's queen bee\n";
        std::cout << "while protecting your own.\n\n";

        // 添加加载动画
        std::cout << "Loading";
        for (int i = 0; i < 3; ++i) {
            std::cout << ".";
            std::cout.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        std::cout << "\n\n";

        resetColor();
    }
    bool Game::getGameMode() {
        while (true) {
            setColor(10); // 绿色
            std::cout << "\nSelect Game Mode:\n";
            resetColor();

            setColor(11); // 亮蓝色
            std::cout << "1. Player vs Player\n";

            setColor(14); // 黄色
            std::cout << "2. Player vs AI\n";

            setColor(15); // 亮白色
            std::cout << "Enter your choice (1-2): ";
            resetColor();

            int choice;
            if (std::cin >> choice) {
                if (choice == 1 || choice == 2) {
                    return choice == 1;
                }
            }

            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            setColor(12); // 红色
            std::cout << "Invalid choice. Please try again.\n";
            resetColor();
        }
    }
    bool Game::getUseExtendedPieces() {
        while (true) {
            setColor(10); // 绿色
            std::cout << "\nWould you like to use the Worm piece?\n";

            setColor(15); // 亮白色
            std::cout << "Worm: Can move 3 spaces under the hive\n";
            std::cout << "and swap positions with insects.\n\n";

            setColor(11); // 亮蓝色
            std::cout << "1. Yes\n";

            setColor(12); // 红色
            std::cout << "2. No\n";

            setColor(15); // 亮白色
            std::cout << "Enter your choice (1-2): ";
            resetColor();

            int choice;
            if (std::cin >> choice) {
                if (choice == 1 || choice == 2) {
                    return choice == 1;
                }
            }

            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            setColor(12); // 红色
            std::cout << "Invalid choice. Please try again.\n";
            resetColor();
        }
    }

    GameCommand Game::getCommand() const {
        int choice;
        std::cout << "\nEnter your choice (1-" << COMMAND_DESCRIPTIONS.size() << "): ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return GameCommand::INVALID;
        }

        if (choice < 1 || choice > static_cast<int>(COMMAND_DESCRIPTIONS.size())) {
            return GameCommand::INVALID;
        }

        return static_cast<GameCommand>(choice - 1);
    }

    void Game::displayMenu() const {
        std::cout << "\n\033[1;36mAvailable Commands:\033[0m\n";
        int i = 1;
        for (const auto& [command, description] : COMMAND_DESCRIPTIONS) {
            // 使用不同颜色显示不同类型的命令
            if (command == GameCommand::PLACE_PIECE || command == GameCommand::MOVE_PIECE) {
                std::cout << "\033[1;32m"; // 绿色显示主要操作
            } else if (command == GameCommand::QUIT) {
                std::cout << "\033[1;31m"; // 红色显示退出
            } else {
                std::cout << "\033[1;34m"; // 蓝色显示其他选项
            }
            std::cout << i++ << ". " << description << "\033[0m\n";
        }
    }

    void Game::displayGameStatus() const {
        setColor(11);
        std::cout << "\n=== Current Game Status ===\n";

        // 显示当前玩家
        setColor(14);
        std::cout << "Current Player: ";
        setColor(15);
        std::cout << currentPlayer->getName();
        if (!currentPlayer->isHuman()) {
            std::cout << " (AI)";
        }
        std::cout << "\n";

        auto stateIt = playerStates.find(currentPlayer->getID());
        if (stateIt != playerStates.end()) {
            const auto& state = stateIt->second;
            setColor(10);
            std::cout << "Turn: " << state.turnCount + 1 << "\n";

            if (state.mustPlaceQueen) {
                setColor(12);
                std::cout << "*** Queen Bee must be placed this turn! ***\n";
            }

            if (!board.isQueenPlaced(currentPlayer->getID())) {
                int turnsRemaining = 3 - std::min(state.turnCount, 3);
                if (turnsRemaining > 0) {
                    setColor(14);
                    std::cout << "Queen Bee not yet placed ("
                             << turnsRemaining
                             << " turns remaining)\n";
                }
            }

            // 显示扩展棋子状态
            if (hasExtendedPieces) {
                setColor(13);
                std::cout << "Extended Piece (Worm) Available: "
                         << (board.piecesAvailable.at(currentPlayer->getID()).at(PieceName::Worm) > 0 ? "Yes" : "No")
                         << "\n";
            }
        }

        resetColor();
        board.printBoard();

        // 如果当前是AI的回合，添加思考提示
        if (!currentPlayer->isHuman()) {
            setColor(14);
            std::cout << "\nAI is thinking";
            for (int i = 0; i < 3; ++i) {
                std::cout << ".";
                std::cout.flush();
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
            std::cout << "\n";
            resetColor();
        }
    }

    void Game::displayHelp() const {
        std::cout << "\n\033[1;36m=== Game Rules ===\033[0m\n"
                  << "\033[1;37m1. Players take turns placing or moving pieces\n"
                  << "2. The Queen Bee must be placed by turn 4\n"
                  << "3. To win, surround the opponent's Queen Bee\n"
                  << "4. Each piece type has unique movement rules:\n"
                  << "   - Queen (Q): Moves one space at a time\n"
                  << "   - Ant (A): Can move any number of spaces around the hive\n"
                  << "   - Spider (S): Must move exactly three spaces\n"
                  << "   - Beetle (B): Moves one space, can climb on top of other pieces\n"
                  << "   - Grasshopper (G): Must jump in a straight line over other pieces\n";

        if (hasExtendedPieces) {
            std::cout << "   - Worm (W): Can move three spaces under the hive and swap positions\n";
        }

        std::cout << "\033[0m\n";
    }

    void Game::displayStats() const {
        std::cout << "\n\033[1;36m=== Game Statistics ===\033[0m\n"
                  << "\033[1;37mTotal Turns: " << stats.turnCount << "\n"
                  << "Pieces Placed: " << stats.piecesPlaced << "\n"
                  << "Player 1 Moves: " << stats.movesMade.at(PlayerID::player1) << "\n"
                  << "Player 2 Moves: " << stats.movesMade.at(PlayerID::player2);

        if (hasExtendedPieces) {
            std::cout << "\nWorm Pieces Used: "
                      << (2 - (board.piecesAvailable.at(PlayerID::player1).at(PieceName::Worm) +
                              board.piecesAvailable.at(PlayerID::player2).at(PieceName::Worm)));
        }

        std::cout << "\033[0m\n";
    }
    void Game::handleGameCommand(GameCommand command) {
        auto stateIt = playerStates.find(currentPlayer->getID());
        if (stateIt == playerStates.end()) {
            throw std::runtime_error("Player state not initialized properly");
        }

        auto& currentPlayerState = stateIt->second;

        try {
            if (currentPlayerState.turnCount >= 3 && !board.isQueenPlaced(currentPlayer->getID())) {
                currentPlayerState.mustPlaceQueen = true;
            }

            switch (command) {
                case GameCommand::PLACE_PIECE: {
                    currentPlayer->makeMove(board, command);
                    currentPlayerState.turnCount++;
                    stats.movesMade[currentPlayer->getID()]++;
                    stats.piecesPlaced++;
                    break;
                }
                case GameCommand::MOVE_PIECE: {
                    if (currentPlayerState.mustPlaceQueen) {
                        throw InvalidMoveException("You must place your Queen Bee before moving pieces!");
                    }
                    if (hasExtendedPieces && board.getTopPiece(HexCoord())->getEumName() == PieceName::Worm) {
                        std::cout << "\033[1;35mMoving a Worm: Remember you can swap positions!\033[0m\n";
                    }
                    currentPlayer->makeMove(board, command);
                    currentPlayerState.turnCount++;
                    stats.movesMade[currentPlayer->getID()]++;
                    break;
                }
                case GameCommand::SHOW_HELP:
                    displayHelp();
                break;
                case GameCommand::SHOW_BOARD:
                    board.printBoard();
                break;
                case GameCommand::SHOW_STATS:
                    displayStats();
                break;
                default:
                    break;
            }
        } catch (const std::exception& e) {
            std::cout << "\033[1;31mError: " << e.what() << "\033[0m\n";
        }
    }


    void Game::forcePlaceQueen() {
        std::cout << "\n\033[1;31mYou must place your Queen Bee. Enter coordinates (x y): \033[0m";
        int x, y;
        while (true) {
            if (!(std::cin >> x >> y)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "\033[1;31mInvalid input. Please enter two numbers: \033[0m";
                continue;
            }

            try {
                auto queen = std::make_shared<QueenBee>(currentPlayer->getID());
                board.addPiece(queen, HexCoord(x, y), currentPlayer->getID());
                std::cout << "\033[1;32mQueen Bee placed at (" << x << "," << y << ")\033[0m\n";
                break;
            } catch (const std::exception& e) {
                std::cout << "\033[1;31mInvalid placement: " << e.what() << "\nTry again: \033[0m";
            }
        }
    }

    void Game::switchPlayer() {
        currentPlayer = (currentPlayer == players[0]) ? players[1] : players[0];
        if (playerStates.find(currentPlayer->getID()) == playerStates.end()) {
            playerStates[currentPlayer->getID()] = PlayerState();
        }
        stats.turnCount++;
        std::cout << "\n\033[1;36mSwitching to " << currentPlayer->getName() << "'s turn\033[0m\n";
    }
    void Game::updateGameState() {
        Victory victory = board.checkVictory();

        // 只在真正达到终局时才结束游戏
        if (victory != Victory::NONE) {
            gameState = GameState::GAME_OVER;
            std::cout << "\nGame Over! ";
            switch (victory) {
                case Victory::PLAYER1_WINS:
                    std::cout << players[0]->getName() << " wins!\n";
                break;
                case Victory::PLAYER2_WINS:
                    std::cout << players[1]->getName() << " wins!\n";
                break;
                case Victory::DRAW:
                    std::cout << "Game ended in a draw!\n";
                break;
                default:
                    break;
            }
        }
    }
    bool Game::isGameOver() const {
        if (gameState == GameState::GAME_OVER) {
            std::cout << "\n\033[1;33m=== Game Over ===\033[0m\n";
            return true;
        }
        return false;
    }



    std::string Game::getWinner() const {
        Victory victory = board.checkVictory();
        std::string result;

        switch (victory) {
            case Victory::PLAYER1_WINS:
                result = "\033[1;32m" + players[0]->getName() + " Wins!\033[0m";
            break;
            case Victory::PLAYER2_WINS:
                result = "\033[1;32m" + players[1]->getName() + " Wins!\033[0m";
            break;
            case Victory::DRAW:
                result = "\033[1;33mDraw - Game ended in a stalemate\033[0m";
            break;
            default:
                result = "\033[1;37mGame in progress\033[0m";
        }
        return result;
    }


    void Game::clearScreen() const {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }
}

