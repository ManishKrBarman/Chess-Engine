#include "Move.h"
#include "Board.h"
#include <sstream>

namespace chess {

std::string Move::toString() const {
    std::ostringstream oss;
    oss << Board::squareToString(from) << Board::squareToString(to);
    if (isPromotion()) {
        char promoChar = 'q';
        switch (promotion) {
            case PieceType::Queen: promoChar = 'q'; break;
            case PieceType::Rook: promoChar = 'r'; break;
            case PieceType::Bishop: promoChar = 'b'; break;
            case PieceType::Knight: promoChar = 'n'; break;
            default: break;
        }
        oss << promoChar;
    }
    return oss.str();
}

} // namespace chess
