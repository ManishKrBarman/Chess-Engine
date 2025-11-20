#pragma once

#include <array>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <cstdint>
#include "Piece.h"
#include "Move.h"

namespace chess {

class Board {
public:
    Board();

    const Piece* pieceAt(Square sq) const { return squares_[sq].get(); }
    Piece* pieceAt(Square sq) { return squares_[sq].get(); }

    Color sideToMove() const { return sideToMove_; }

    bool whiteKingside() const { return whiteKingside_; }
    bool whiteQueenside() const { return whiteQueenside_; }
    bool blackKingside() const { return blackKingside_; }
    bool blackQueenside() const { return blackQueenside_; }

    Square enPassantSquare() const { return enPassantSquare_; }

    void reset(); 

    std::string toFEN() const; 
    void fromFEN(const std::string& fen);

    void applyMove(Move& m); 
    void undoMove(const Move& m);

    bool inCheck(Color c) const;
    bool isSquareAttacked(Square sq, Color by) const;

    void generatePseudoLegalMoves(std::vector<Move>& out) const;
    void generateLegalMoves(std::vector<Move>& out);

    uint64_t perft(int depth);

    static std::string squareToString(Square sq);
    static Square parseSquare(const std::string& s);

private:
    std::array<std::unique_ptr<Piece>, 64> squares_{}; 
    Color sideToMove_ { Color::White };
    bool whiteKingside_ { true };
    bool whiteQueenside_ { true };
    bool blackKingside_ { true };
    bool blackQueenside_ { true };
    Square enPassantSquare_ { -1 };
    int halfmoveClock_ { 0 }; 
    int fullmoveNumber_ { 1 };

    void placeInitialPieces();
};

}
