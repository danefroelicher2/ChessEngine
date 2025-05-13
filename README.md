# Chess Engine

A simple chess engine written in C++ with a command-line interface. This project provides a foundation for developing a more advanced chess engine with machine learning capabilities.

## Features

- Complete chess rules implementation including:
  - All standard piece movements
  - Castling
  - En passant
  - Pawn promotion
  - Check and checkmate detection
  - Stalemate detection
  - Draw by insufficient material
  - Draw by fifty-move rule
  - Draw by threefold repetition
- Simple AI engine using alpha-beta pruning minimax algorithm
- Command-line user interface
- Support for FEN notation
- Adjustable engine search depth

## Building the Project

### Prerequisites

- C++17 compatible compiler
- CMake (version 3.10 or higher)

### Build Instructions

1. Clone the repository:
```bash
git clone https://github.com/yourusername/chess-engine.git
cd chess-engine
```

2. Create a build directory and navigate to it:
```bash
mkdir build
cd build
```

3. Generate the build files with CMake:
```bash
cmake ..
```

4. Build the project:
```bash
cmake --build .
```

5. Run the chess engine:
```bash
./chess_engine
```

## Usage

Once the chess engine is running, you can use the following commands:

- `help` - Display the help message
- `new` - Start a new game (you play as white)
- `new black` - Start a new game (you play as black)
- `fen [fen]` - Start a game from the given FEN position
- `print` - Print the current board position
- `undo` - Undo the last move
- `resign` - Resign the current game
- `draw` - Offer a draw
- `depth [n]` - Set the engine search depth to n
- `quit` or `exit` - Exit the program

To make a move, enter the source and destination squares. For example: `e2e4` moves the piece from e2 to e4.
For pawn promotion, add q, r, b, or n at the end. For example: `e7e8q` promotes to a queen.

## Future Enhancements

- Graphical user interface
- Opening book
- Endgame tablebase
- Machine learning integration
- Multi-threading for improved performance
- UCI protocol support for compatibility with chess GUIs

## License

This project is licensed under the MIT License - see the LICENSE file for details.