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

#include "GameConstants.h"

namespace piecetype {

// Forward declarations
class Player;
class Board;


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
    private:
        struct PlayerState {
            int turnCount = 0;
            bool mustPlaceQueen = false;
        };

        Board board;
        std::vector<std::shared_ptr<Player>> players;
        std::shared_ptr<Player> currentPlayer;
        GameState gameState;
        GameStats stats;
        std::map<PlayerID, PlayerState> playerStates;

        static const int BOARD_SIZE = 2;

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

    public:
        Game();
        void start();

        const Board& getBoard() const {
            return board;
        }
        // 2. 非const版本返回普通引用
        Board& getBoard() {
            return board;
        }
    };


}

#endif //GAME_H
