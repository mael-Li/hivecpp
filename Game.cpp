//
// Created by 李世佳 on 24-10-17.
//
#include "Game.h"
using namespace piecetype;
int getMenuChoice() {
    std::vector<std::string> options = {"Place Piece", "Move Piece", "Exit"};
    int choice = -1;

    while (choice != 2) { // 2对应"Move Piece"
        std::cout << "\nHive Game Menu:\n";
        for (size_t i = 0; i < options.size(); ++i) {
            std::cout << i + 1 << ". " << options[i] << "\n";
        }
        std::cout << "Choose an option: ";
        if (!(std::cin >> choice) || choice < 1 || choice > static_cast<int>(options.size())) {
            std::cout << "Invalid input, please try again.\n";
            std::cin.clear(); // Reset failbit
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore rest of line
            continue;
        }
        switch (choice) {
            case 1: // Place Piece
                std::cout << "Placing a piece...\n";
            return 1;
            case 2: // Move Piece
                std::cout << "Moving a piece...\n";
            return 2;
            case 3: // Exit
                std::cout << "Exiting the game...\n";
            return 0;
            default:
                std::cout << "Unsupported option.\n";
        }
    }
    return 0;
}
Game::Game() : board(10) {
    // 初始化玩家
    players.push_back(std::make_shared<HumanPlayer>("Human 1",PlayerID::player1));
    players.push_back(std::make_shared<HumanPlayer>("Human 2",PlayerID::player2));
}
void Game::start() {
    bool gameIsOver = false;
    std::string winner;
    int currentPlayerIndex = 0; // 当前玩家索引
    while (!gameIsOver) {
        auto currentPlayer = players[currentPlayerIndex];
        //检测胜利与否
        if (board.isQueenBeeSurround(currentPlayer->getid())) {
            gameIsOver = true;
            winner = getOpponentName(currentPlayer->getid());
            std::cout << "Player " << currentPlayer->getName()
                      << " has been surrounded! The winner is " << winner << "!" << std::endl;
            break;
        }
        //显示哪个玩家的回合
        std::cout << "It's " << currentPlayer->getName() << "'s turn." << std::endl;
        // 显示菜单并获取用户的选择
        int commandOrder = getMenuChoice();
        // 在makeMove函数内处理异常输入
        try {
            currentPlayer->makeMove(board, commandOrder);
        } catch (const std::exception& e) {
            std::cout << "An error occurred: " << e.what()
                      << " Please try your action again." << std::endl;
            continue; // 继续当前玩家的回合
        }
        std::cout << "Current board state:" << std::endl;
        board.printBoard();
        // 切换到下一个玩家
        currentPlayerIndex = (currentPlayerIndex + 1) % players.size();
        index++;
    }
}

HumanPlayer::HumanPlayer(std::string n,const PlayerID&a):Player(std::move(n),a){}

void HumanPlayer::makeMove(Board& board, const int c) {
    oneaction();
    bool command_move = false;
    std::string command;
    std::string player;
    std::string pieceType;
    if(getid() == PlayerID::player1) player = "Player 1";
    else player = "Player 2";
    //将命令导入函数
    if(c == 1) command = "place";
    else if(c == 2) command = "move";
    //判断
    if (command == "place") {
        std::cout << player << ", please enter your type of :";
        std::cin >> pieceType;
        int x, y;
        std::cout<<"please enter your position of piece:";
        std::cin>>x>>y;
        std::cout<<std::endl;
        // 创建棋子实例
        std::shared_ptr<Piece> piece;
        if (pieceType == "Q") {
            HexCoord pos(x,y);
            piece = std::make_shared<QueenBee>(getid());
        } else if (pieceType == "A") {
            piece = std::make_shared<Ant>(getid());
        }
        else if(pieceType == "S"){
            piece = std::make_shared<Spider>(getid());
        }else if(pieceType == "B"){
            piece = std::make_shared<Beetle>(getid());
        }else if(pieceType == "G"){
            piece = std::make_shared<Grasshopper>(getid());
        }
        else{
            throw Pieceexception("Unknown piece type.");
        }
        // 放置棋子
        board.addPiece(piece, HexCoord(x, y),getid());
    } else if (command == "move") {
        int fromX, fromY, toX, toY;
        while (!command_move) {
            std::cout << "Please enter the position you want to move (x y): ";
            std::cin >> fromX >> fromY;
            if (std::cin.fail()) {
                std::cin.clear(); // 清除错误标志
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 忽略错误输入直到下一个换行符
                std::cerr << "Invalid input. Please enter integers only." << std::endl;
                continue; // 重新开始循环
            }

            auto piece = board.getPieceAt(HexCoord(fromX, fromY));
            if (!piece) {
                throw Pieceexception("No piece at the given coordinate.");
            }
            command_move = true; // 找到棋子，标记输入为有效，并退出循环
        }
        auto piece = board.getPieceAt(HexCoord(fromX, fromY));
        std::cout << "Please enter the new position you want to move to (x y): ";
        if (std::cin >> toX >> toY && std::cin.good()) { // 检查输入的有效性
            piece->move(board, HexCoord(toX, toY), getid());
        } else {
            std::cin.clear(); // 清除错误标志
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 忽略错误输入直到下一个换行符
            std::cerr << "Invalid input. Please enter integers only." << std::endl;
            // 可能需要重新提示用户输入或者做其他处理
        }
    }
    else {
        std::cout << "Invalid command." << std::endl;
    }
}

AIPlayer::AIPlayer(std::string n,const PlayerID&):Player(std::move(n),PlayerID::playerai){}

void AIPlayer::makeMove(piecetype::Board& board,int) {
    // AI 的逻辑来决定移动
    /*
    index_ai++;
    int command = index_ai;
    std::cout << getName() << " is making a move..." << std::endl;
    // 示例移动逻辑
    switch(command) {
        case 0:
            auto queen = std::make_shared<QueenBee>(PlayerID::playerai);
            int x,y;
            std::mt19937 generator(std::random_device{}());
            std::uniform_int_distribution<> distribution(-board.getSize(), board.getSize());
            x = distribution(generator);
            y = distribution(generator);
            board.addPiece(queen,HexCoord(x,y), PlayerID::playerai);
        case 1:
            auto ant = std::make_shared<Ant>(PlayerID::playerai);
            std::mt19937 generator1(std::random_device{}());
            std::uniform_int_distribution<> distribution1(-board.getSize(), board.getSize());
            x = distribution1(generator);
            y = distribution1(generator);
            board.addPiece(queen,HexCoord(x,y), PlayerID::playerai);
            index_ai =0;
        default:
            std::cout << "Unknown command." << std::endl;
        break;
    }

    auto queen = std::make_shared<QueenBee>(PlayerID::playerai);
    int x,y;
    std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<> distribution(-board.getSize(), board.getSize());
    x = distribution(generator);
    y = distribution(generator);
    board.addPiece(queen,HexCoord(x,y), PlayerID::playerai);
    */
    auto pieces = board.getAllPiecesOnBoard(board.getSize());
    if (!pieces.empty()) {
        auto piece = pieces.front();
        piece->move(board, HexCoord(1, 1),PlayerID::playerai); // 示例移动
    }

}
