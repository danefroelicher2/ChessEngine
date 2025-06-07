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

void UI::runPerftTests()
{
    PerftTester::runTestSuite();
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
    try
    {
        // STEP 1: Basic input validation
        if (command.empty())
        {
            return true; // Ignore empty commands silently
        }

        // STEP 2: Trim whitespace and validate
        std::string trimmedCommand = command;
        // Remove leading/trailing whitespace
        size_t start = trimmedCommand.find_first_not_of(" \t\n\r");
        if (start == std::string::npos)
        {
            return true; // Only whitespace
        }
        size_t end = trimmedCommand.find_last_not_of(" \t\n\r");
        trimmedCommand = trimmedCommand.substr(start, end - start + 1);

        // STEP 3: Log command for debugging (optional)
        // std::cout << "Processing command: '" << trimmedCommand << "'" << std::endl;

        // STEP 4: Process commands with individual error handling
        if (trimmedCommand == "quit" || trimmedCommand == "exit")
        {
            gameActive = false;
            return false;
        }
        else if (trimmedCommand == "help")
        {
            try
            {
                displayHelp();
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error displaying help: " << e.what() << std::endl;
            }
        }
        else if (trimmedCommand == "test" || trimmedCommand == "tactics")
        {
            try
            {
                std::cout << "Running tactical test suite..." << std::endl;
                runTacticalTests();
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error running tactical tests: " << e.what() << std::endl;
            }
        }
        else if (trimmedCommand == "new")
        {
            try
            {
                newGame();
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error starting new game: " << e.what() << std::endl;
            }
        }
        else if (trimmedCommand == "perft")
        {
            try
            {
                std::cout << "Running enhanced perft test suite..." << std::endl;
                PerftTester::runTestSuite();
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error running perft tests: " << e.what() << std::endl;
            }
        }
        else if (trimmedCommand == "benchmark" || trimmedCommand == "bench")
        {
            try
            {
                std::cout << "Running perft performance benchmark suite..." << std::endl;
                PerftTester::runBenchmarkSuite();
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error running benchmark tests: " << e.what() << std::endl;
            }
        }
        else if (trimmedCommand == "regression")
        {
            try
            {
                std::cout << "Running performance regression test..." << std::endl;
                PerftTester::runPerformanceRegression();
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error running regression test: " << e.what() << std::endl;
            }
        }
        else if (trimmedCommand == "perft basic")
        {
            try
            {
                std::cout << "Running basic perft tests..." << std::endl;
                runPerftTests();
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error running basic perft tests: " << e.what() << std::endl;
            }
        }
        else if (trimmedCommand == "new black")
        {
            try
            {
                newGame(false);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error starting new game as black: " << e.what() << std::endl;
            }
        }
        else if (trimmedCommand.substr(0, 4) == "fen ")
        {
            try
            {
                std::string fenString = trimmedCommand.substr(4);
                if (fenString.empty())
                {
                    std::cout << "Error: No FEN string provided. Usage: fen <fen-string>" << std::endl;
                    std::cout << "Example: fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" << std::endl;
                }
                else
                {
                    std::cout << "Loading FEN: " << fenString << std::endl;
                    newGameFromFEN(fenString);
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error loading FEN position: " << e.what() << std::endl;
            }
        }
        else if (trimmedCommand == "print")
        {
            try
            {
                printGame();
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error printing game: " << e.what() << std::endl;
            }
        }
        else if (trimmedCommand == "undo")
        {
            try
            {
                if (game.undoMove())
                {
                    // Undo twice if playing against the engine (undo both player and engine moves)
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
            catch (const std::exception &e)
            {
                std::cerr << "Error undoing move: " << e.what() << std::endl;
            }
        }
        else if (trimmedCommand == "resign")
        {
            try
            {
                game.endInDrawByAgreement(); // For now, treat resign as draw
                std::cout << "Game ended by resignation." << std::endl;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error resigning: " << e.what() << std::endl;
            }
        }
        else if (trimmedCommand == "draw")
        {
            try
            {
                game.endInDrawByAgreement();
                std::cout << "Game ended by draw agreement." << std::endl;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error offering draw: " << e.what() << std::endl;
            }
        }
        else if (trimmedCommand.substr(0, 6) == "depth ")
        {
            try
            {
                int depth = std::stoi(trimmedCommand.substr(6));
                if (depth > 0 && depth <= 20)
                {
                    engine.setDepth(depth);
                    std::cout << "Engine search depth set to " << depth << std::endl;
                }
                else
                {
                    std::cout << "Invalid depth! Must be between 1 and 20." << std::endl;
                }
            }
            catch (const std::exception &e)
            {
                std::cout << "Invalid depth format! Usage: depth <number>" << std::endl;
            }
        }
        else if (trimmedCommand.substr(0, 7) == "ttsize ")
        {
            try
            {
                int sizeMB = std::stoi(trimmedCommand.substr(7));
                if (sizeMB > 0 && sizeMB <= 2048)
                {
                    engine.setTTSize(sizeMB);
                    std::cout << "Transposition table size set to " << sizeMB << " MB" << std::endl;
                }
                else
                {
                    std::cout << "Invalid size! Must be between 1 and 2048 MB." << std::endl;
                }
            }
            catch (const std::exception &e)
            {
                std::cout << "Invalid size format! Usage: ttsize <megabytes>" << std::endl;
            }
        }
        else if (trimmedCommand == "cleartt")
        {
            try
            {
                engine.clearTT();
                std::cout << "Transposition table cleared" << std::endl;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error clearing transposition table: " << e.what() << std::endl;
            }
        }
        else if (trimmedCommand.substr(0, 5) == "time ")
        {
            try
            {
                int timeMs = std::stoi(trimmedCommand.substr(5));
                if (timeMs > 0 && timeMs <= 3600000)
                { // Max 1 hour
                    engine.setTimeForMove(timeMs);
                    std::cout << "Time per move set to " << timeMs << " milliseconds" << std::endl;
                }
                else
                {
                    std::cout << "Invalid time! Must be between 1 and 3600000 ms." << std::endl;
                }
            }
            catch (const std::exception &e)
            {
                std::cout << "Invalid time format! Usage: time <milliseconds>" << std::endl;
            }
        }
        else if (trimmedCommand == "time off")
        {
            try
            {
                engine.setTimeManagement(false);
                std::cout << "Time management disabled" << std::endl;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error disabling time management: " << e.what() << std::endl;
            }
        }
        else if (trimmedCommand.substr(0, 6) == "perft ")
        {
            try
            {
                int depth = std::stoi(trimmedCommand.substr(6));
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
                std::cout << "Invalid perft depth! Usage: perft <depth>" << std::endl;
            }
        }
        else if (trimmedCommand == "tuning")
        {
            try
            {
                std::cout << "Running automated parameter tuning..." << std::endl;
                engine.runParameterTuning("standard");
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error running parameter tuning: " << e.what() << std::endl;
            }
        }
        else if (trimmedCommand.substr(0, 6) == "abtest")
        {
            try
            {
                if (trimmedCommand.length() > 7)
                {
                    std::string params = trimmedCommand.substr(7);
                    size_t spacePos = params.find(' ');
                    if (spacePos != std::string::npos)
                    {
                        std::string paramName = params.substr(0, spacePos);
                        int newValue = std::stoi(params.substr(spacePos + 1));

                        bool success = engine.runABTest(paramName, newValue, 10);
                        std::cout << "A/B Test " << (success ? "PASSED" : "FAILED") << std::endl;
                    }
                }
                else
                {
                    std::cout << "Usage: abtest <parameter_name> <new_value>" << std::endl;
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error running A/B test: " << e.what() << std::endl;
            }
        }
        else if (trimmedCommand == "testimprovements")
        {
            try
            {
                std::cout << "Testing search improvements..." << std::endl;

                // Test tactical position
                game.newGameFromFEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

                auto startTime = std::chrono::high_resolution_clock::now();
                Move bestMove = engine.getBestMove();
                auto endTime = std::chrono::high_resolution_clock::now();

                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

                std::cout << "Best move: " << bestMove.toString() << std::endl;
                std::cout << "Nodes searched: " << engine.getNodesSearched() << std::endl;
                std::cout << "Time taken: " << duration.count() << "ms" << std::endl;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error testing improvements: " << e.what() << std::endl;
            }
        }
        else if (trimmedCommand == "eval")
        {
            try
            {
                std::cout << "=== Detailed Position Evaluation ===" << std::endl;
                Board board = game.getBoard();
                
                // Show overall evaluation
                int totalEval = engine.evaluatePosition(board);
                std::cout << "Total Evaluation: " << totalEval << " (from " 
                          << (board.getSideToMove() == Color::WHITE ? "White's" : "Black's") 
                          << " perspective)" << std::endl;
                
                std::cout << "\n--- Component Breakdown ---" << std::endl;
                
                // Test each component
                std::cout << "Piece Mobility: " << engine.evaluatePieceMobility(board) << std::endl;
                std::cout << "King Safety: " << engine.evaluateKingSafety(board) << std::endl;
                std::cout << "Pawn Structure: " << engine.evaluatePawnStructure(board) << std::endl;
                std::cout << "Piece Coordination: " << engine.evaluatePieceCoordination(board) << std::endl;
                
                if (engine.isEndgame(board)) {
                    std::cout << "Endgame Factors: " << engine.evaluateEndgameFactors(board) << std::endl;
                }
                
                std::cout << "\n--- Detailed Analysis ---" << std::endl;
                std::cout << "White Mobility: " << engine.countPieceMobility(board, Color::WHITE) << std::endl;
                std::cout << "Black Mobility: " << engine.countPieceMobility(board, Color::BLACK) << std::endl;
                std::cout << "White Pawn Islands: " << engine.getPawnIslands(board, Color::WHITE) << std::endl;
                std::cout << "Black Pawn Islands: " << engine.getPawnIslands(board, Color::BLACK) << std::endl;
                std::cout << "White Bishop Pair: " << (engine.hasBishopPair(board, Color::WHITE) ? "Yes" : "No") << std::endl;
                std::cout << "Black Bishop Pair: " << (engine.hasBishopPair(board, Color::BLACK) ? "Yes" : "No") << std::endl;
                std::cout << "Is Endgame: " << (engine.isEndgame(board) ? "Yes" : "No") << std::endl;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error evaluating position: " << e.what() << std::endl;
            }
        }
        else if (trimmedCommand == "evaltest")
        {
            try
            {
                std::cout << "=== Testing Evaluation on Known Positions ===" << std::endl;
                
                // Test starting position
                game.newGameFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
                std::cout << "Starting Position: " << engine.evaluatePosition(game.getBoard()) << std::endl;
                
               // Test position with pawn structure issues
               game.newGameFromFEN("rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 2");
               std::cout << "Symmetrical Pawn Structure: " << engine.evaluatePosition(game.getBoard()) << std::endl;
               
               // Test endgame position
               game.newGameFromFEN("8/8/8/8/8/4k3/4P3/4K3 w - - 0 1");
               std::cout << "King & Pawn Endgame: " << engine.evaluatePosition(game.getBoard()) << std::endl;
               
               // Test bishop pair position
               game.newGameFromFEN("rnbqk1nr/pppp1ppp/8/2b1p3/2B1P3/8/PPPP1PPP/RNBQK1NR w KQkq - 4 4");
               std::cout << "Italian Game (Bishop Development): " << engine.evaluatePosition(game.getBoard()) << std::endl;
               
               // Restore original position
               game.newGame();
           }
           catch (const std::exception &e)
           {
               std::cerr << "Error in evaluation test: " << e.what() << std::endl;
           }
       }
       else if (trimmedCommand == "uci")
        {
            try
            {
                std::cout << "=== Testing UCI Protocol ===" << std::endl;
                std::cout << "Switching to UCI mode..." << std::endl;
                std::cout << "Note: This will change to UCI protocol." << std::endl;
                std::cout << "Send 'uci' command to start UCI session." << std::endl;
                
                // For testing, we'll create a temporary UCI instance
                UCIProtocol uci(game, engine);
                
                // Test some UCI commands
                std::cout << "\nTesting UCI identification:" << std::endl;
                uci.processCommand("uci");
                
                std::cout << "\nTesting ready status:" << std::endl;
                uci.processCommand("isready");
                
                std::cout << "\nUCI test complete. Use './chess_engine --uci' for full UCI mode." << std::endl;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error testing UCI: " << e.what() << std::endl;
            }
        }
        else
        {
            // STEP 5: Try to interpret the command as a move
            if (isPlayerTurn())
            {
                try
                {
                    if (game.makeMove(trimmedCommand))
                    {
                        printGame();
                    }
                    else
                    {
                        std::cout << "Invalid move! Type 'help' for assistance." << std::endl;
                        std::cout << "Make sure to use format like: e2e4, g1f3, e7e8q (for promotion)" << std::endl;
                    }
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Error making move: " << e.what() << std::endl;
                    std::cout << "Type 'help' for move format assistance." << std::endl;
                }
            }
            else
            {
                std::cout << "Unknown command: '" << trimmedCommand << "'. Type 'help' for available commands." << std::endl;
            }
        }

        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error processing command '" << command << "': " << e.what() << std::endl;
        std::cout << "Type 'help' for available commands." << std::endl;
        return true; // Continue running despite error
    }
    catch (...)
    {
        std::cerr << "Unknown error processing command '" << command << "'" << std::endl;
        std::cout << "Type 'help' for available commands." << std::endl;
        return true; // Continue running despite error
    }
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
    std::cout << "  tuning         - Run automated parameter tuning" << std::endl;
    std::cout << "  abtest [param] [value] - Run A/B test for parameter" << std::endl;
    std::cout << "  testimprovements - Test search improvements on tactical position" << std::endl;
    std::cout << "  time [ms]      - Set time per move in milliseconds" << std::endl;
    std::cout << "  time off       - Disable time management" << std::endl;
    std::cout << std::endl;
    std::cout << "  perft          - Run perft test suite" << std::endl;
    std::cout << "  perft [n]      - Run perft to depth n on current position" << std::endl;
    std::cout << "To make a move, enter the source and destination squares." << std::endl;
    std::cout << "For example: e2e4 moves the piece from e2 to e4." << std::endl;
    std::cout << "For pawn promotion, add q, r, b, or n at the end." << std::endl;
    std::cout << "For example: e7e8q promotes to a queen." << std::endl;
    std::cout << "  perft          - Run perft test suite" << std::endl;
    std::cout << "  benchmark      - Run performance benchmark suite" << std::endl;
    std::cout << "  regression     - Run quick performance regression test" << std::endl;
    std::cout << "  eval           - Show detailed position evaluation" << std::endl;
    std::cout << "  evaltest       - Test evaluation on known positions" << std::endl;
    std::cout << "  uci            - Test UCI protocol commands" << std::endl;
}