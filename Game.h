//
// Created by 李世佳 on 24-10-17.
//
#ifndef GAME_H
#define GAME_H
#include "Hive.h"
#include <utility>
#include <vector>
#include "Hive.h"
#include <utility>
#include <vector>
#include <memory>
#include <map>
#include <functional>

namespace piecetype {

// Forward declarations
class Player;
class Board;

enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};

enum class GameCommand {
    PLACE_PIECE,
    MOVE_PIECE,
    SHOW_HELP,
    SHOW_BOARD,
    SHOW_STATS,
    QUIT,
    INVALID
};

struct GameStats {
    int turnCount = 0;
    int piecesPlaced = 0;
    std::map<PlayerID, int> movesMade;
    std::map<PlayerID, bool> queenPlaced;
};

class Player {
public:
    Player(std::string n, const PlayerID& a) : name(std::move(n)), id(a) {}
    virtual ~Player() = default;

    virtual void makeMove(Board& board, GameCommand command) = 0;
    virtual bool isHuman() const = 0;

    const std::string& getName() const { return name; }
    PlayerID getID() const { return id; }
    int getMoveCount() const { return moveCount; }
    void incrementMoveCount() { moveCount++; }

protected:
    std::string name;
    PlayerID id;
    int moveCount = 0;
};

class HumanPlayer : public Player {
public:
    HumanPlayer(const std::string& name, const PlayerID& id);
    void makeMove(Board& board, GameCommand command) override;
    bool isHuman() const override { return true; }

private:
    void handlePlacePiece(Board& board);
    void handleMovePiece(Board& board);
    std::shared_ptr<Piece> createPiece(const std::string& pieceType);
    bool validateCoordinates(int x, int y, const Board& board) const;
    void placePiece(Board& board, const std::string& pieceType);
};

    class Game {
    public:
        Game();
        void start();

    private:
        struct PlayerState {
            int turnCount = 0;
            bool mustPlaceQueen = false;
        };

        void initializeGame();
        void gameLoop();

        void displayStats() const;

        void handleGameCommand(GameCommand command);
        void switchPlayer();

        void forcePlaceQueen();

        void updateGameState();

        bool isGameOver() const;

        void displayGameStatus() const;

        std::string getWinner() const;

        void clearScreen() const;

        void displayHelp() const;

        void displayMenu() const;

        GameCommand getCommand() const;
        Board board;
        std::vector<std::shared_ptr<Player>> players;
        std::shared_ptr<Player> currentPlayer;
        GameState gameState;
        GameStats stats;
        std::map<PlayerID, PlayerState> playerStates;  // 玩家状态映射
        static const std::map<GameCommand, std::string> COMMAND_DESCRIPTIONS;
        static const int BOARD_SIZE = 10;
    };
// Constants for piece types and their symbols
const std::map<std::string, PieceName> PIECE_TYPES = {
    {"Q", PieceName::Queen},
    {"A", PieceName::Ant},
    {"S", PieceName::Spider},
    {"B", PieceName::Beetle},
    {"G", PieceName::Grasshopper}
};

}

#endif //GAME_H
