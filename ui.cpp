#include "ui.h"
#include <iostream>
#include <chrono>
#include <thread>

void UI::newGame(bool playerPlaysWhite)
{
    game.newGame();
    playerIsWhite = playerPlaysWhite;
    gameActive = true;
    printGame();
}

void UI::newGameFromFEN(const std::string &fen, bool playerPlaysWhite)
{
    game.newGameFromFEN(fen);
    playerIsWhite = playerPlaysWhite;
    gameActive = true;
    printGame();
}
void UI::runTacticalTests()
{
    TacticalTester::runTestSuite(engine);
}
void UI::run()
{
    std::cout << "Welcome to the Chess Engine!" << std::endl;
    std::cout << "Type 'help' for a list of commands." << std::endl;

    // Start a new game by default
    newGame();

    // Main game loop
    while (gameActive)
    {
        // Check if the game is over
        if (game.isGameOver())
        {
            std::cout << "Game over! ";

            switch (game.getResult())
            {
            case GameResult::WHITE_WINS:
                std::cout << "White wins";
                break;
            case GameResult::BLACK_WINS:
                std::cout << "Black wins";
                break;
            case GameResult::DRAW:
                std::cout << "Draw";
                break;
            default:
                break;
            }

            switch (game.getEndReason())
            {
            case GameEndReason::CHECKMATE:
                std::cout << " by checkmate";
                break;
            case GameEndReason::STALEMATE:
                std::cout << " by stalemate";
                break;
            case GameEndReason::INSUFFICIENT_MATERIAL:
                std::cout << " by insufficient material";
                break;
            case GameEndReason::FIFTY_MOVE_RULE:
                std::cout << " by fifty-move rule";
                break;
            case GameEndReason::THREEFOLD_REPETITION:
                std::cout << " by threefold repetition";
                break;
            case GameEndReason::AGREEMENT:
                std::cout << " by agreement";
                break;
            default:
                break;
            }

            std::cout << std::endl;
            std::cout << "Type 'new' to start a new game or 'quit' to exit." << std::endl;

            // Get player command
            std::string command;
            std::getline(std::cin, command);

            if (!processCommand(command))
            {
                break;
            }

            continue;
        }

        // Player's turn
        if (isPlayerTurn())
        {
            std::cout << "Your turn " << (playerIsWhite ? "(White)" : "(Black)") << "." << std::endl;
            std::cout << "Enter a move or command: ";

            // Get player input
            std::string input;
            std::getline(std::cin, input);

            // Check if it's a command
            if (!processCommand(input))
            {
                break;
            }
        }
        // Engine's turn
        else
        {
            std::cout << "Engine is thinking..." << std::endl;

            // Simulate thinking time
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            // Get engine move
            Move move = getEngineMove();

            // Make the move
            if (game.makeMove(move))
            {
                std::cout << "Engine plays: " << move.toString() << std::endl;
                printGame();
            }
            else
            {
                std::cerr << "Error: Engine made an invalid move!" << std::endl;
                gameActive = false;
            }
        }
    }

    std::cout << "Thanks for playing!" << std::endl;
}

void UI::printGame() const
{
    game.print();
}

Move UI::getPlayerMove() const
{
    std::string moveStr;
    std::cout << "Enter your move (e.g., e2e4): ";
    std::getline(std::cin, moveStr);

    // Parse the move
    if (moveStr.length() < 4)
    {
        return Move(Position(), Position());
    }

    Position from = Position::fromString(moveStr.substr(0, 2));
    Position to = Position::fromString(moveStr.substr(2, 2));

    if (!from.isValid() || !to.isValid())
    {
        return Move(Position(), Position());
    }

    // Check for promotion
    PieceType promotion = PieceType::NONE;
    if (moveStr.length() > 4)
    {
        char promChar = moveStr[4];
        switch (promChar)
        {
        case 'q':
            promotion = PieceType::QUEEN;
            break;
        case 'r':
            promotion = PieceType::ROOK;
            break;
        case 'b':
            promotion = PieceType::BISHOP;
            break;
        case 'n':
            promotion = PieceType::KNIGHT;
            break;
        default:
            break;
        }
    }

    return Move(from, to, promotion);
}

Move UI::getEngineMove()
{
    return engine.getBestMove();
}

bool UI::processCommand(const std::string &command)
{
    if (command == "quit" || command == "exit")
    {
        gameActive = false;
        return false;
    }
    else if (command == "help")
    {
        displayHelp();
    }
    else if (command == "test" || command == "tactics")
    {
        runTacticalTests();
    }
    else if (command == "new")
    {
        newGame();
    }
    else if (command == "new black")
    {
        newGame(false);
    }
    else if (command.substr(0, 4) == "fen ")
    {
        newGameFromFEN(command.substr(4));
    }
    else if (command == "print")
    {
        printGame();
    }
    else if (command == "undo")
    {
        if (game.undoMove())
        {
            // Undo twice if playing against the engine
            if (!game.isGameOver() && !isPlayerTurn())
            {
                game.undoMove();
            }
            printGame();
        }
        else
        {
            std::cout << "Cannot undo move!" << std::endl;
        }
    }
    else if (command == "resign")
    {
        if (playerIsWhite)
        {
            game.endInDrawByAgreement();
        }
        else
        {
            game.endInDrawByAgreement();
        }
    }
    else if (command == "draw")
    {
        game.endInDrawByAgreement();
    }
    else if (command.substr(0, 6) == "depth ")
    {
        try
        {
            int depth = std::stoi(command.substr(6));
            if (depth > 0)
            {
                engine.setDepth(depth);
                std::cout << "Engine search depth set to " << depth << std::endl;
            }
            else
            {
                std::cout << "Invalid depth!" << std::endl;
            }
        }
        catch (const std::exception &e)
        {
            std::cout << "Invalid depth!" << std::endl;
        }
    }
    else if (command.substr(0, 7) == "ttsize ")
    {
        try
        {
            int sizeMB = std::stoi(command.substr(7));
            if (sizeMB > 0)
            {
                engine.setTTSize(sizeMB);
                std::cout << "Transposition table size set to " << sizeMB << " MB" << std::endl;
            }
            else
            {
                std::cout << "Invalid size!" << std::endl;
            }
        }
        catch (const std::exception &e)
        {
            std::cout << "Invalid size!" << std::endl;
        }
    }
    else if (command == "cleartt")
    {
        engine.clearTT();
        std::cout << "Transposition table cleared" << std::endl;
    }
    else if (command == "perft")
    {
        runPerftTests();
    }
    else if (command.substr(0, 6) == "perft ")
    {
        try
        {
            int depth = std::stoi(command.substr(6));
            if (depth > 0 && depth <= 6)
            {
                Board board = game.getBoard();
                PerftTester::perftDivide(board, depth);
            }
            else
            {
                std::cout << "Perft depth must be between 1 and 6!" << std::endl;
            }
        }
        catch (const std::exception &e)
        {
            std::cout << "Invalid perft depth!" << std::endl;
        }
    }
    else
    {
        // Try to interpret the command as a move
        if (isPlayerTurn())
        {
            if (game.makeMove(command))
            {
                printGame();
            }
            else
            {
                std::cout << "Invalid move! Type 'help' for assistance." << std::endl;
            }
        }
    }

    return true;
}

bool UI::isPlayerTurn() const
{
    return (game.getBoard().getSideToMove() == Color::WHITE && playerIsWhite) ||
           (game.getBoard().getSideToMove() == Color::BLACK && !playerIsWhite);
}

void UI::displayHelp() const
{
    std::cout << "Available commands:" << std::endl;
    std::cout << "  help           - Display this help message" << std::endl;
    std::cout << "  new            - Start a new game (you play as white)" << std::endl;
    std::cout << "  new black      - Start a new game (you play as black)" << std::endl;
    std::cout << "  fen [fen]      - Start a game from the given FEN position" << std::endl;
    std::cout << "  print          - Print the current board position" << std::endl;
    std::cout << "  undo           - Undo the last move" << std::endl;
    std::cout << "  resign         - Resign the current game" << std::endl;
    std::cout << "  draw           - Offer a draw" << std::endl;
    std::cout << "  depth [n]      - Set the engine search depth to n" << std::endl;
    std::cout << "  ttsize [n]     - Set the transposition table size to n MB" << std::endl;
    std::cout << "  cleartt        - Clear the transposition table" << std::endl;
    std::cout << "  quit/exit      - Exit the program" << std::endl;
    std::cout << "  test/tactics   - Run tactical test suite" << std::endl;
    std::cout << std::endl;
    std::cout << "  perft          - Run perft test suite" << std::endl;
    std::cout << "  perft [n]      - Run perft to depth n on current position" << std::endl;
    std::cout << "To make a move, enter the source and destination squares." << std::endl;
    std::cout << "For example: e2e4 moves the piece from e2 to e4." << std::endl;
    std::cout << "For pawn promotion, add q, r, b, or n at the end." << std::endl;
    std::cout << "For example: e7e8q promotes to a queen." << std::endl;

    void UI::runPerftTests()
    {
        PerftTester::runTestSuite();
    }
}