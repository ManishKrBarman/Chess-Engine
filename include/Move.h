#pragma once
/**\n * Move representation capturing sufficient state for undo operations.\n */
#include <cstdint>
#include <string>

namespace chess {

// Board squares are indexed 0..63 (file + rank*8).
using Square = int; // -1 means invalid

enum class Color : uint8_t { White, Black };

// Piece types for evaluation / identification
enum class PieceType : uint8_t { None, King, Queen, Rook, Bishop, Knight, Pawn };

// Bit flags for special move attributes
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
    PieceType promotion { PieceType::None }; // Promotion piece type (Queen by default)
    uint16_t flags { NORMAL }; // Bitmask of MoveFlags
    PieceType captured { PieceType::None }; // Captured piece type (for undo info)

    // Previous state to restore on undo
    bool prevWhiteKingside { true };
    bool prevWhiteQueenside { true };
    bool prevBlackKingside { true };
    bool prevBlackQueenside { true };
    Square prevEnPassant { -1 }; // previous en-passant square
    int prevHalfmoveClock { 0 }; // for 50-move rule (placeholder)

    Move() = default;
    Move(Square f, Square t, uint16_t fl = NORMAL, PieceType promo = PieceType::None)
        : from(f), to(t), promotion(promo), flags(fl) {}

    bool isCapture() const { return flags & CAPTURE; }
    bool isPromotion() const { return flags & PROMOTION; }
    bool isCastle() const { return flags & CASTLING; }
    bool isEnPassant() const { return flags & EN_PASSANT; }

    std::string toString() const; // Algebraic coordinate string (e2e4, with suffix for promo)
};

} // namespace chess
