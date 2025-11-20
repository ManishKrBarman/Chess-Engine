#pragma once

#include <cstdint>
#include <string>

namespace chess {

using Square = int; 

enum class Color : uint8_t { White, Black };

// Piece types
enum class PieceType : uint8_t { None, King, Queen, Rook, Bishop, Knight, Pawn };

// Special moves
enum MoveFlags : uint16_t {
    NORMAL = 0,
    CAPTURE = 1 << 0,
    PROMOTION = 1 << 1,
    CASTLING = 1 << 2,
    EN_PASSANT = 1 << 3
};

struct Move {
    Square from { -1 };
    Square to { -1 };
    PieceType promotion { PieceType::None }; // queen default
    uint16_t flags { NORMAL }; 
    PieceType captured { PieceType::None }; // for undo info

    bool prevWhiteKingside { true };
    bool prevWhiteQueenside { true };
    bool prevBlackKingside { true };
    bool prevBlackQueenside { true };
    Square prevEnPassant { -1 }; 
    int prevHalfmoveClock { 0 }; 

    Move() = default;
    Move(Square f, Square t, uint16_t fl = NORMAL, PieceType promo = PieceType::None)
        : from(f), to(t), promotion(promo), flags(fl) {}

    bool isCapture() const { return flags & CAPTURE; }
    bool isPromotion() const { return flags & PROMOTION; }
    bool isCastle() const { return flags & CASTLING; }
    bool isEnPassant() const { return flags & EN_PASSANT; }

    std::string toString() const; 
};

}
