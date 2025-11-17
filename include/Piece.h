#pragma once
/**
 * Piece base class and derived piece types.
 * Each derived piece implements generateMoves() to produce pseudo-legal moves.
 */
#include <vector>
#include <memory>
#include <string>
#include "Move.h"

namespace chess {

class Board; // forward declaration

class Piece {
public:
    Piece(Color color, PieceType type) : color_(color), type_(type) {}
    virtual ~Piece() = default;

    Color color() const { return color_; }
    PieceType type() const { return type_; }

    // Returns pseudo-legal moves for this piece from a square on a board
    virtual void generateMoves(const Board& board, Square from, std::vector<Move>& out) const = 0;

    // single character symbol for printing
    virtual char symbol() const = 0;

protected:
    Color color_;
    PieceType type_;
};

// Concrete pieces
class King : public Piece {
public:
    King(Color c) : Piece(c, PieceType::King) {}
    void generateMoves(const Board& board, Square from, std::vector<Move>& out) const override;
    char symbol() const override { return color_ == Color::White ? 'K' : 'k'; }
};

class Queen : public Piece {
public:
    Queen(Color c) : Piece(c, PieceType::Queen) {}
    void generateMoves(const Board& board, Square from, std::vector<Move>& out) const override;
    char symbol() const override { return color_ == Color::White ? 'Q' : 'q'; }
};

class Rook : public Piece {
public:
    Rook(Color c) : Piece(c, PieceType::Rook) {}
    void generateMoves(const Board& board, Square from, std::vector<Move>& out) const override;
    char symbol() const override { return color_ == Color::White ? 'R' : 'r'; }
};

class Bishop : public Piece {
public:
    Bishop(Color c) : Piece(c, PieceType::Bishop) {}
    void generateMoves(const Board& board, Square from, std::vector<Move>& out) const override;
    char symbol() const override { return color_ == Color::White ? 'B' : 'b'; }
};

class Knight : public Piece {
public:
    Knight(Color c) : Piece(c, PieceType::Knight) {}
    void generateMoves(const Board& board, Square from, std::vector<Move>& out) const override;
    char symbol() const override { return color_ == Color::White ? 'N' : 'n'; }
};

class Pawn : public Piece {
public:
    Pawn(Color c) : Piece(c, PieceType::Pawn) {}
    void generateMoves(const Board& board, Square from, std::vector<Move>& out) const override;
    char symbol() const override { return color_ == Color::White ? 'P' : 'p'; }
};

} // namespace chess
