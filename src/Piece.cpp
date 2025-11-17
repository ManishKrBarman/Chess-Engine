#include "Piece.h"
#include "Board.h"

namespace chess {

static inline int fileOf(Square s){ return s % 8; }
static inline int rankOf(Square s){ return s / 8; }
static inline bool onBoard(int f, int r){ return f>=0 && f<8 && r>=0 && r<8; }
static inline Square toSq(int f,int r){ return r*8+f; }

// Helper to add move if target square not occupied by friendly piece
static void addIfValid(const Board& board, const Piece* pc, Square from, int tf, int tr, std::vector<Move>& out){
    if(!onBoard(tf,tr)) return; Square to = toSq(tf,tr); const Piece* tgt = board.pieceAt(to); if(tgt){ if(tgt->color()==pc->color()) return; Move m{from,to, (uint16_t)(tgt? CAPTURE: NORMAL)}; m.captured = tgt? tgt->type(): PieceType::None; out.push_back(m);} else { out.push_back(Move{from,to}); } }

void King::generateMoves(const Board& board, Square from, std::vector<Move>& out) const {
    for(int df=-1; df<=1; ++df){ for(int dr=-1; dr<=1; ++dr){ if(df||dr) addIfValid(board,this,from,fileOf(from)+df,rankOf(from)+dr,out);} }
    auto squaresClear = [&](const std::vector<Square>& squares){
        for(Square sq : squares){ if(board.pieceAt(sq)) return false; }
        return true;
    };
    if(color_==Color::White){
        if(board.whiteKingside()){
            if(squaresClear({toSq(5,0), toSq(6,0)})) out.push_back(Move{from, toSq(6,0), CASTLING});
        }
        if(board.whiteQueenside()){
            if(squaresClear({toSq(1,0), toSq(2,0), toSq(3,0)})) out.push_back(Move{from,toSq(2,0), CASTLING});
        }
    } else {
        if(board.blackKingside()){
            if(squaresClear({toSq(5,7), toSq(6,7)})) out.push_back(Move{from, toSq(6,7), CASTLING});
        }
        if(board.blackQueenside()){
            if(squaresClear({toSq(1,7), toSq(2,7), toSq(3,7)})) out.push_back(Move{from,toSq(2,7), CASTLING});
        }
    }
}

void Queen::generateMoves(const Board& board, Square from, std::vector<Move>& out) const {
    static const int dirs[8][2]={{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}}; for(auto &d:dirs){ int f=fileOf(from)+d[0], r=rankOf(from)+d[1]; while(onBoard(f,r)){ Square to=toSq(f,r); const Piece* tgt=board.pieceAt(to); if(tgt){ if(tgt->color()!=color_) { Move m{from,to,(uint16_t)CAPTURE}; m.captured=tgt->type(); out.push_back(m);} break; } out.push_back(Move{from,to}); f+=d[0]; r+=d[1]; }} }

void Rook::generateMoves(const Board& board, Square from, std::vector<Move>& out) const {
    static const int dirs[4][2]={{1,0},{-1,0},{0,1},{0,-1}}; for(auto &d:dirs){ int f=fileOf(from)+d[0], r=rankOf(from)+d[1]; while(onBoard(f,r)){ Square to=toSq(f,r); const Piece* tgt=board.pieceAt(to); if(tgt){ if(tgt->color()!=color_) { Move m{from,to,(uint16_t)CAPTURE}; m.captured=tgt->type(); out.push_back(m);} break; } out.push_back(Move{from,to}); f+=d[0]; r+=d[1]; }} }

void Bishop::generateMoves(const Board& board, Square from, std::vector<Move>& out) const {
    static const int dirs[4][2]={{1,1},{1,-1},{-1,1},{-1,-1}}; for(auto &d:dirs){ int f=fileOf(from)+d[0], r=rankOf(from)+d[1]; while(onBoard(f,r)){ Square to=toSq(f,r); const Piece* tgt=board.pieceAt(to); if(tgt){ if(tgt->color()!=color_) { Move m{from,to,(uint16_t)CAPTURE}; m.captured=tgt->type(); out.push_back(m);} break; } out.push_back(Move{from,to}); f+=d[0]; r+=d[1]; }} }

void Knight::generateMoves(const Board& board, Square from, std::vector<Move>& out) const {
    static const int jumps[8][2]={{2,1},{2,-1},{-2,1},{-2,-1},{1,2},{1,-2},{-1,2},{-1,-2}}; for(auto &j:jumps){ addIfValid(board,this,from,fileOf(from)+j[0],rankOf(from)+j[1],out);} }

void Pawn::generateMoves(const Board& board, Square from, std::vector<Move>& out) const {
    int dir = (color_==Color::White)?1:-1; int f=fileOf(from), r=rankOf(from); int fr=r+dir; if(onBoard(f,fr)){ Square forward=toSq(f,fr); if(!board.pieceAt(forward)){ // move forward
        bool promotion = (fr==0 || fr==7); if(promotion){ Move m{from,forward,(uint16_t)PROMOTION,PieceType::Queen}; out.push_back(m);} else { out.push_back(Move{from,forward}); }
        // double move
        if((color_==Color::White && r==1) || (color_==Color::Black && r==6)){ int fr2=r+2*dir; if(onBoard(f,fr2) && !board.pieceAt(toSq(f,fr2))){ out.push_back(Move{from,toSq(f,fr2)}); }} }
    }
    // captures
    for(int df=-1; df<=1; df+=2){ int cf=f+df; int cr=r+dir; if(onBoard(cf,cr)){ Square to=toSq(cf,cr); const Piece* tgt=board.pieceAt(to); if(tgt && tgt->color()!=color_){ bool promotion=(cr==0||cr==7); Move m{from,to,(uint16_t)(CAPTURE | (promotion? PROMOTION:0)), promotion?PieceType::Queen:PieceType::None}; m.captured=tgt->type(); out.push_back(m);} }
    }
    // en-passant capture handled in Board::generatePseudoLegalMoves (flag insertion)
}

} // namespace chess
