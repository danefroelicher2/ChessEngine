#ifndef PIECE_H
#define PIECE_H

#include "main.h"

enum class PieceType {
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    NONE
};

enum class Color {
    WHITE,
    BLACK,
    NONE
};

struct Position {
    int row;
    int col;
    
    Position(int r = -1, int c = -1) : row(r), col(c) {}
    
    bool isValid() const {
        return row >= 0 && row < 8 && col >= 0 && col < 8;
    }
    
    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }
    
    std::string toString() const {
        if (!isValid()) return "invalid";
        char file = 'a' + col;
        char rank = '1' + row;
        return std::string(1, file) + std::string(1, rank);
    }
    
    static Position fromString(const std::string& str) {
        if (str.length() != 2) return Position();
        int col = str[0] - 'a';
        int row = str[1] - '1';
        return Position(row, col);
    }
};

class Move {
public:
    Position from;
    Position to;
    PieceType promotion;
    
    Move(Position f, Position t, PieceType p = PieceType::NONE)
        : from(f), to(t), promotion(p) {}
    
    std::string toString() const {
        std::string result = from.toString() + to.toString();
        if (promotion != PieceType::NONE) {
            char promChar = ' ';
            switch (promotion) {
                case PieceType::QUEEN: promChar = 'q'; break;
                case PieceType::ROOK: promChar = 'r'; break;
                case PieceType::BISHOP: promChar = 'b'; break;
                case PieceType::KNIGHT: promChar = 'n'; break;
                default: break;
            }
            result += promChar;
        }
        return result;
    }
};

class Piece {
protected:
    PieceType type;
    Color color;
    Position position;
    bool hasMoved;

public:
    Piece(PieceType t, Color c, Position pos)
        : type(t), color(c), position(pos), hasMoved(false) {}
    
    virtual ~Piece() = default;
    
    PieceType getType() const { return type; }
    Color getColor() const { return color; }
    Position getPosition() const { return position; }
    bool getHasMoved() const { return hasMoved; }
    
    void setPosition(const Position& pos) { position = pos; }
    void setMoved() { hasMoved = true; }
    
    virtual std::vector<Move> getLegalMoves(const class Board& board) const = 0;
    
    // Helper to check if move is on board and doesn't capture own piece
    bool isBasicallyValid(const Position& pos, const Board& board) const;
    
    // Return char representation for console display
    char toChar() const;
};

#endif // PIECE_H