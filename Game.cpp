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
        {"G", PieceName::Grasshopper}
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
        bool queenRequired = (board.getTurnCount() >= 6 && !board.isQueenPlaced(getID()));

        if (queenRequired) {
            std::cout << "\nYou must place your Queen Bee this turn!\n";
            placePiece(board, "Q");
            return;
        }

        // 显示可用棋子
        std::cout << "\nAvailable pieces to place:\n";
        for (const auto& [symbol, type] : PIECE_TYPES) {
            int remaining = board.piecesAvailable[getID()][type];
            if (remaining > 0) {
                std::cout << symbol << " (" << remaining << " remaining)\n";
            }
        }

        // 获取玩家选择的棋子类型
        std::string pieceType;
        bool validPieceType = false;
        do {
            std::cout << "\nEnter piece type (Q/A/S/B/G): ";
            std::cin >> pieceType;
            std::transform(pieceType.begin(), pieceType.end(), pieceType.begin(), ::toupper);

            if (PIECE_TYPES.find(pieceType) != PIECE_TYPES.end()) {
                if (board.piecesAvailable[getID()][PIECE_TYPES.at(pieceType)] > 0) {
                    validPieceType = true;
                } else {
                    std::cout << "No more " << pieceType << " pieces available.\n";
                }
            } else {
                std::cout << "Invalid piece type. Please try again.\n";
            }
        } while (!validPieceType);

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

    Game::Game() : board(BOARD_SIZE), gameState(GameState::MENU) {
        initializeGame();
    }

    void Game::initializeGame() {
        // 创建玩家
        players.clear();
        players.push_back(std::make_shared<HumanPlayer>("Player 1", PlayerID::player1));
        players.push_back(std::make_shared<HumanPlayer>("Player 2", PlayerID::player2));

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
            displayMenu();

            GameCommand command = getCommand();
            if (command == GameCommand::QUIT) {
                break;
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
                std::cout << "Press Enter to continue...";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
        }

        if (isGameOver()) {
            std::cout << "\nGame Over! " << getWinner() << " wins!\n";
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
        std::cout << "\nAvailable Commands:\n";
        int i = 1;
        for (const auto& [command, description] : COMMAND_DESCRIPTIONS) {
            std::cout << i++ << ". " << description << "\n";
        }
    }

    void Game::displayGameStatus() const {
        std::cout << "\n=== Current Game Status ===\n";
        std::cout << "Current Player: " << currentPlayer->getName() << "\n";

        // 安全地访问玩家状态
        auto stateIt = playerStates.find(currentPlayer->getID());
        if (stateIt != playerStates.end()) {
            const auto& state = stateIt->second;
            std::cout << "Turn: " << state.turnCount + 1 << "\n";

            if (state.mustPlaceQueen) {
                std::cout << "*** Queen Bee must be placed this turn! ***\n";
            }

            if (!board.isQueenPlaced(currentPlayer->getID())) {
                int turnsRemaining = 3 - std::min(state.turnCount, 3);
                if (turnsRemaining > 0) {
                    std::cout << "Queen Bee not yet placed ("
                              << turnsRemaining
                              << " turns remaining)\n";
                }
            }
        }

        board.printBoard();
    }

    void Game::displayHelp() const {
        std::cout << "\n=== Game Rules ===\n"
                  << "1. Players take turns placing or moving pieces\n"
                  << "2. The Queen Bee must be placed by turn 4\n"
                  << "3. To win, surround the opponent's Queen Bee\n"
                  << "4. Each piece type has unique movement rules\n\n";
    }

    void Game::displayStats() const {
        std::cout << "\n=== Game Statistics ===\n"
                  << "Total Turns: " << stats.turnCount << "\n"
                  << "Pieces Placed: " << stats.piecesPlaced << "\n"
                  << "Player 1 Moves: " << stats.movesMade.at(PlayerID::player1) << "\n"
                  << "Player 2 Moves: " << stats.movesMade.at(PlayerID::player2) << "\n";
    }

    void Game::handleGameCommand(GameCommand command) {
        // 安全地获取当前玩家状态
        auto stateIt = playerStates.find(currentPlayer->getID());
        if (stateIt == playerStates.end()) {
            throw std::runtime_error("Player state not initialized properly");
        }

        auto& currentPlayerState = stateIt->second;

        try {
            // 检查是否必须放置蜂后
            if (currentPlayerState.turnCount >= 3 && !board.isQueenPlaced(currentPlayer->getID())) {
                currentPlayerState.mustPlaceQueen = true;
            }

            switch (command) {
                case GameCommand::PLACE_PIECE: {
                    currentPlayer->makeMove(board, command);
                    currentPlayerState.turnCount++;
                    stats.movesMade[currentPlayer->getID()]++;
                    break;
                }
                case GameCommand::MOVE_PIECE: {
                    if (currentPlayerState.mustPlaceQueen) {
                        throw InvalidMoveException("You must place your Queen Bee before moving pieces!");
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
            std::cout << "Error: " << e.what() << "\n";
        }
    }

    void Game::forcePlaceQueen() {
        std::cout << "\nYou must place your Queen Bee. Enter coordinates (x y): ";
        int x, y;
        while (true) {
            if (!(std::cin >> x >> y)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input. Please enter two numbers: ";
                continue;
            }

            try {
                auto queen = std::make_shared<QueenBee>(currentPlayer->getID());
                board.addPiece(queen, HexCoord(x, y), currentPlayer->getID());
                std::cout << "Queen Bee placed at (" << x << "," << y << ")\n";
                break;
            } catch (const std::exception& e) {
                std::cout << "Invalid placement: " << e.what() << "\nTry again: ";
            }
        }
    }

    void Game::switchPlayer() {
        // 切换当前玩家
        currentPlayer = (currentPlayer == players[0]) ? players[1] : players[0];

        // 安全检查新的当前玩家状态是否存在
        if (playerStates.find(currentPlayer->getID()) == playerStates.end()) {
            playerStates[currentPlayer->getID()] = PlayerState();
        }

        stats.turnCount++;
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
        return gameState == GameState::GAME_OVER;
    }


    std::string Game::getWinner() const {
        Victory victory = board.checkVictory();
        switch (victory) {
            case Victory::PLAYER1_WINS:
                return players[0]->getName();
            case Victory::PLAYER2_WINS:
                return players[1]->getName();
            case Victory::DRAW:
                return "Draw - Game ended in a stalemate";
            default:
                return "Game in progress";
        }
    }
    void Game::clearScreen() const {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }
}

