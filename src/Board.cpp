#include "Board.h"
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <cctype>

namespace chess {

static inline int fileOf(Square s){ return s % 8; }
static inline int rankOf(Square s){ return s / 8; }
static inline Square toSq(int f,int r){ return r*8+f; }
static inline bool onBoard(int f,int r){ return f>=0 && f<8 && r>=0 && r<8; }

Board::Board(){ reset(); }

void Board::placeInitialPieces(){
    // Clear
    for(auto &p: squares_) p.reset();
    // Pawns
    for(int f=0; f<8; ++f){ squares_[toSq(f,1)] = std::make_unique<Pawn>(Color::White); squares_[toSq(f,6)] = std::make_unique<Pawn>(Color::Black); }
    // Rooks
    squares_[toSq(0,0)] = std::make_unique<Rook>(Color::White); squares_[toSq(7,0)] = std::make_unique<Rook>(Color::White);
    squares_[toSq(0,7)] = std::make_unique<Rook>(Color::Black); squares_[toSq(7,7)] = std::make_unique<Rook>(Color::Black);
    // Knights
    squares_[toSq(1,0)] = std::make_unique<Knight>(Color::White); squares_[toSq(6,0)] = std::make_unique<Knight>(Color::White);
    squares_[toSq(1,7)] = std::make_unique<Knight>(Color::Black); squares_[toSq(6,7)] = std::make_unique<Knight>(Color::Black);
    // Bishops
    squares_[toSq(2,0)] = std::make_unique<Bishop>(Color::White); squares_[toSq(5,0)] = std::make_unique<Bishop>(Color::White);
    squares_[toSq(2,7)] = std::make_unique<Bishop>(Color::Black); squares_[toSq(5,7)] = std::make_unique<Bishop>(Color::Black);
    // Queens
    squares_[toSq(3,0)] = std::make_unique<Queen>(Color::White); squares_[toSq(3,7)] = std::make_unique<Queen>(Color::Black);
    // Kings
    squares_[toSq(4,0)] = std::make_unique<King>(Color::White); squares_[toSq(4,7)] = std::make_unique<King>(Color::Black);
}

void Board::reset(){
    placeInitialPieces();
    sideToMove_ = Color::White;
    whiteKingside_ = whiteQueenside_ = blackKingside_ = blackQueenside_ = true;
    enPassantSquare_ = -1;
    halfmoveClock_ = 0; fullmoveNumber_ = 1;
}

std::string Board::toFEN() const {
    std::ostringstream fen;
    for(int r=7; r>=0; --r){ int empty=0; for(int f=0; f<8; ++f){ const Piece* p = pieceAt(toSq(f,r)); if(!p){ ++empty; } else { if(empty){ fen<<empty; empty=0;} fen<<p->symbol(); } } if(empty) fen<<empty; if(r) fen<<'/'; }
    fen << ' ' << (sideToMove_==Color::White?'w':'b') << ' ';
    std::string castle=""; if(whiteKingside_) castle+='K'; if(whiteQueenside_) castle+='Q'; if(blackKingside_) castle+='k'; if(blackQueenside_) castle+='q'; if(castle.empty()) castle="-"; fen<<castle<<' ';
    if(enPassantSquare_==-1) fen<<'-'; else fen<<squareToString(enPassantSquare_);
    fen << ' ' << halfmoveClock_ << ' ' << fullmoveNumber_;
    return fen.str();
}

void Board::fromFEN(const std::string& fen){
    reset();
    std::istringstream iss(fen); std::string boardPart, stm, castle, eps; int half, full; iss>>boardPart>>stm>>castle>>eps>>half>>full; if(iss.fail()) throw std::runtime_error("Invalid FEN");
    for(auto &p: squares_) p.reset();
    int r=7,f=0; for(char c: boardPart){ if(c=='/'){ r--; f=0; continue;} if(std::isdigit(c)){ f+= c-'0'; continue;} Color col = (std::isupper(c)? Color::White: Color::Black); char lc=std::tolower(c); std::unique_ptr<Piece> piece; switch(lc){ case 'p': piece=std::make_unique<Pawn>(col); break; case 'n': piece=std::make_unique<Knight>(col); break; case 'b': piece=std::make_unique<Bishop>(col); break; case 'r': piece=std::make_unique<Rook>(col); break; case 'q': piece=std::make_unique<Queen>(col); break; case 'k': piece=std::make_unique<King>(col); break; default: throw std::runtime_error("Bad piece char"); } squares_[toSq(f,r)] = std::move(piece); f++; }
    sideToMove_ = (stm=="w"? Color::White: Color::Black);
    whiteKingside_ = castle.find('K')!=std::string::npos; whiteQueenside_ = castle.find('Q')!=std::string::npos; blackKingside_ = castle.find('k')!=std::string::npos; blackQueenside_ = castle.find('q')!=std::string::npos;
    enPassantSquare_ = (eps=="-"? -1: parseSquare(eps));
    halfmoveClock_ = half; fullmoveNumber_ = full;
}

std::string Board::squareToString(Square sq){ std::string s; s.push_back('a'+fileOf(sq)); s.push_back('1'+rankOf(sq)); return s; }
Square Board::parseSquare(const std::string& s){ if(s.size()!=2) return -1; int f=s[0]-'a'; int r=s[1]-'1'; if(f<0||f>7||r<0||r>7) return -1; return toSq(f,r);} 

bool Board::isSquareAttacked(Square sq, Color by) const {
    // Naive: generate pseudo moves of side 'by' and see if target included.
    for(int i=0;i<64;++i){ const Piece* p=pieceAt(i); if(!p||p->color()!=by) continue; std::vector<Move> tmp; p->generateMoves(*this,i,tmp); for(auto &m: tmp){ if(m.to==sq){ // Need special handling for pawns (no forward non-captures)
            return true; } }
    }
    return false;
}

bool Board::inCheck(Color c) const {
    // find king square
    for(int i=0;i<64;++i){ const Piece* p=pieceAt(i); if(p && p->type()==PieceType::King && p->color()==c){ return isSquareAttacked(i, c==Color::White? Color::Black: Color::White); }}
    return false; // Should not happen
}

void Board::generatePseudoLegalMoves(std::vector<Move>& out) const {
    out.clear();
    for(int i=0;i<64;++i){ const Piece* p=pieceAt(i); if(!p||p->color()!=sideToMove_) continue; p->generateMoves(*this,i,out); }
    // Add pawn en-passant captures
    if(enPassantSquare_!=-1){ int epFile=fileOf(enPassantSquare_), epRank=rankOf(enPassantSquare_); int targetRank = (sideToMove_==Color::White? 4:3); if(epRank==targetRank){ int pawnRank = (sideToMove_==Color::White? 4:3); for(int df=-1; df<=1; df+=2){ int f=epFile+df; if(onBoard(f,pawnRank)){ Square from=toSq(f,pawnRank); const Piece* p=pieceAt(from); if(p && p->type()==PieceType::Pawn && p->color()==sideToMove_){ Move m{from,enPassantSquare_, (uint16_t)(CAPTURE|EN_PASSANT)}; m.captured = PieceType::Pawn; out.push_back(m); } } } }
    }
}

void Board::generateLegalMoves(std::vector<Move>& out) {
    std::vector<Move> pseudo; generatePseudoLegalMoves(pseudo); out.clear();
    Color us = sideToMove_;
    for(const auto& base : pseudo){
        Move tmp = base; // apply on a temporary move to capture prev-state
        applyMove(tmp);
        if(!inCheck(us)){
            out.push_back(base);
        }
        undoMove(tmp);
    }
}

void Board::applyMove(Move& m){
    // Store previous state
    m.prevWhiteKingside = whiteKingside_; m.prevWhiteQueenside = whiteQueenside_; m.prevBlackKingside = blackKingside_; m.prevBlackQueenside = blackQueenside_; m.prevEnPassant = enPassantSquare_; m.prevHalfmoveClock = halfmoveClock_;

    Piece* moving = pieceAt(m.from); if(!moving) throw std::runtime_error("No piece on from-square");

    // Handle capture
    if(m.isCapture()){
        if(m.isEnPassant()){
            int dir = (sideToMove_==Color::White? -1: 1); Square capSq = toSq(fileOf(m.to), rankOf(m.to)+dir); if(!pieceAt(capSq)) throw std::runtime_error("Invalid en-passant"); m.captured = pieceAt(capSq)->type(); squares_[capSq].reset();
        } else {
            if(!pieceAt(m.to)) throw std::runtime_error("Capture target empty"); m.captured = pieceAt(m.to)->type(); squares_[m.to].reset();
        }
    }

    // Move piece (handle castling rook move)
    if(m.isCastle()){
        int kingFrom = m.from; int kingTo = m.to; int rank = rankOf(kingFrom);
        bool kingside = fileOf(kingTo)==6;
        int rookFrom = kingside? toSq(7,rank): toSq(0,rank);
        int rookTo   = kingside? toSq(5,rank): toSq(3,rank);
        squares_[rookTo] = std::move(squares_[rookFrom]);
    }

    squares_[m.to] = std::move(squares_[m.from]);

    // Promotion
    if(m.isPromotion()){
        Color col = sideToMove_; squares_[m.to].reset(); squares_[m.to] = std::make_unique<Queen>(col); // auto queen
    }

    // Update castling rights simplistic
    if(moving->type()==PieceType::King){ if(sideToMove_==Color::White){ whiteKingside_=whiteQueenside_=false; } else { blackKingside_=blackQueenside_=false; }}
    if(moving->type()==PieceType::Rook){ int f=fileOf(m.from), r=rankOf(m.from); if(r==0 && f==0) whiteQueenside_=false; if(r==0 && f==7) whiteKingside_=false; if(r==7 && f==0) blackQueenside_=false; if(r==7 && f==7) blackKingside_=false; }
    if(m.isCapture() && m.captured==PieceType::Rook){ int f=fileOf(m.to), r=rankOf(m.to); if(r==0 && f==0) whiteQueenside_=false; if(r==0 && f==7) whiteKingside_=false; if(r==7 && f==0) blackQueenside_=false; if(r==7 && f==7) blackKingside_=false; }

    // En-passant square
    enPassantSquare_ = -1; if(moving->type()==PieceType::Pawn){ int fromRank=rankOf(m.from); int toRank=rankOf(m.to); if(std::abs(toRank - fromRank)==2){ enPassantSquare_ = toSq(fileOf(m.from), (fromRank+toRank)/2); }}

    sideToMove_ = (sideToMove_==Color::White? Color::Black: Color::White);
    if(sideToMove_==Color::White) fullmoveNumber_++;
}

void Board::undoMove(const Move& m){
    sideToMove_ = (sideToMove_==Color::White? Color::Black: Color::White); if(sideToMove_==Color::Black) fullmoveNumber_--;
    // Undo piece move and special moves
    if(m.isCastle()){
        int kingFrom = m.from; int kingTo = m.to; int rank = rankOf(kingFrom); bool kingside = fileOf(kingTo)==6; int rookFrom = kingside? toSq(7,rank): toSq(0,rank); int rookTo   = kingside? toSq(5,rank): toSq(3,rank);
        squares_[rookFrom] = std::move(squares_[rookTo]);
    }
    squares_[m.from] = std::move(squares_[m.to]);
    // Restore captured piece
    if(m.isCapture()){
        if(m.isEnPassant()){
            int dir = (sideToMove_==Color::White? 1: -1); Square capSq = toSq(fileOf(m.to), rankOf(m.to)+dir); squares_[capSq] = std::make_unique<Pawn>(sideToMove_==Color::White? Color::Black: Color::White); // assume pawn
        } else {
            // recreate captured piece type (simplified: create generic piece of captured type)
            Color captColor = (sideToMove_==Color::White? Color::Black: Color::White);
            switch(m.captured){
                case PieceType::Pawn: squares_[m.to] = std::make_unique<Pawn>(captColor); break;
                case PieceType::Knight: squares_[m.to] = std::make_unique<Knight>(captColor); break;
                case PieceType::Bishop: squares_[m.to] = std::make_unique<Bishop>(captColor); break;
                case PieceType::Rook: squares_[m.to] = std::make_unique<Rook>(captColor); break;
                case PieceType::Queen: squares_[m.to] = std::make_unique<Queen>(captColor); break;
                case PieceType::King: squares_[m.to] = std::make_unique<King>(captColor); break;
                default: break;
            }
        }
    } else {
        squares_[m.to].reset();
    }
    // TODO: reverse promotion properly (simplified: if promotion flag, assume pawn moved)
    if(m.isPromotion()){
        squares_[m.from].reset(); squares_[m.from] = std::make_unique<Pawn>(sideToMove_); // restore pawn
    }
    whiteKingside_ = m.prevWhiteKingside; whiteQueenside_ = m.prevWhiteQueenside; blackKingside_ = m.prevBlackKingside; blackQueenside_ = m.prevBlackQueenside; enPassantSquare_ = m.prevEnPassant; halfmoveClock_ = m.prevHalfmoveClock;
}

uint64_t Board::perft(int depth){
    if(depth==0) return 1; std::vector<Move> moves; generateLegalMoves(moves); uint64_t nodes=0; for(auto &mv: moves){ Move backup=mv; applyMove(mv); nodes += perft(depth-1); undoMove(backup); } return nodes;
}

} // namespace chess
