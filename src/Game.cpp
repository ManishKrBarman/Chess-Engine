#include "Game.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cctype>

namespace chess {

Game::Game() : logFilePath_("game_log.txt") { newGame(); }

void Game::newGame(){
    board_.reset();
    moveHistory_.clear();
    pgnMoves_.clear();
    actionLog_.clear();
    resetLogFile();
    logAction("New game started.");
}

void Game::printBoard() const {
    std::cout << "\n  +-----------------+\n";
    for(int r=7; r>=0; --r){ std::cout << r+1 << " | "; for(int f=0; f<8; ++f){ int sq = r*8+f; const Piece* p = board_.pieceAt(sq);
        char c = p? p->symbol() : '.'; std::cout << c << ' '; }
        std::cout << "|\n"; }
    std::cout << "  +-----------------+\n    a b c d e f g h\n";
    std::cout << (board_.sideToMove()==Color::White? "White":"Black") << " to move\n";
}

bool Game::makeMoveString(const std::string& moveStr, bool record){
    if(moveStr.size()<4) throw std::runtime_error("Enter moves like e2e4 or e7e8q for promotion");
    Square from = Board::parseSquare(moveStr.substr(0,2)); Square to = Board::parseSquare(moveStr.substr(2,2)); if(from==-1||to==-1) throw std::runtime_error("Bad square");
    std::vector<Move> legal; board_.generateLegalMoves(legal);
    for(auto &m: legal){ if(m.from==from && m.to==to){ if(moveStr.size()==5){ char pc=std::tolower(moveStr[4]); switch(pc){ case 'q': m.promotion=PieceType::Queen; m.flags|=PROMOTION; break; case 'r': m.promotion=PieceType::Rook; m.flags|=PROMOTION; break; case 'b': m.promotion=PieceType::Bishop; m.flags|=PROMOTION; break; case 'n': m.promotion=PieceType::Knight; m.flags|=PROMOTION; break; default: throw std::runtime_error("Bad promotion piece"); } }
            const Piece* movingPiece = board_.pieceAt(from);
            PieceType movingType = movingPiece? movingPiece->type(): PieceType::None;
            Color mover = board_.sideToMove();
            Move backup=m;
            board_.applyMove(m);
            if(record){
                recordMove(backup, mover, movingType);
            }
            return true; } }
    throw std::runtime_error("Illegal move");
}

std::string Game::moveToAlgebraic(const Move& m) const {
    // Notation: e2e4, with x and promotion suffix
    std::string s = Board::squareToString(m.from) + (m.isCapture()?"x":"") + Board::squareToString(m.to);
    if(m.isPromotion()){
        char c='Q'; switch(m.promotion){ case PieceType::Rook: c='R'; break; case PieceType::Bishop: c='B'; break; case PieceType::Knight: c='N'; break; default: c='Q'; }
        s.push_back('='); s.push_back(c);
    }
    return s;
}

void Game::save(const std::string& filename) const { std::ofstream ofs(filename); if(!ofs) throw std::runtime_error("Cannot open file for save"); ofs << board_.toFEN() << "\n"; }
void Game::load(const std::string& filename){ std::ifstream ifs(filename); if(!ifs) throw std::runtime_error("Cannot open file for load"); std::string fen; std::getline(ifs, fen); board_.fromFEN(fen); moveHistory_.clear(); pgnMoves_.clear(); actionLog_.clear(); resetLogFile(); logAction("Loaded position from " + filename + "."); }

void Game::exportPGN(const std::string& filename) const {
    std::ofstream ofs(filename); if(!ofs) throw std::runtime_error("Cannot open PGN file");
    ofs << "[Event \"Casual Game\"]\n[Site \"Local\"]\n[Date \"????.??.??\"]\n[Round \"-\"]\n[White \"Player1\"]\n[Black \"Player2\"]\n[Result \"*\"]\n\n";
    for(size_t i=0;i<pgnMoves_.size();++i){ if(i%2==0) ofs << (i/2+1) << ". "; ofs << pgnMoves_[i] << ' '; }
    ofs << "*\n";
}

void Game::loop(){
    std::cout << "Welcome to ChessEngine! Type 'help' for commands.\n";
    configureMode(false);
    printBoard();
    maybePlayComputerTurn();
    std::string cmd;
    while(true){
        std::cout << "> ";
        if(!std::getline(std::cin, cmd)) break;
        cmd = trim(cmd);
        if(cmd.empty()) continue;
        try{
            if(cmd=="help"){ std::cout << "Commands: move (e2e4), save FILE, load FILE, pgn FILE, undo, new, board, log, quit\n"; continue; }
            if(cmd=="board"){ printBoard(); continue; }
            if(cmd=="quit"||cmd=="exit") break;
            if(cmd=="new"){ newGame(); configureMode(true); printBoard(); maybePlayComputerTurn(); continue; }
            if(cmd.rfind("save ",0)==0){ save(cmd.substr(5)); std::cout<<"Saved.\n"; continue; }
            if(cmd.rfind("load ",0)==0){ load(cmd.substr(5)); configureMode(true); printBoard(); maybePlayComputerTurn(); continue; }
            if(cmd.rfind("pgn ",0)==0){ exportPGN(cmd.substr(4)); std::cout<<"PGN exported.\n"; continue; }
            if(cmd=="undo"){ if(moveHistory_.empty()){ std::cout<<"Nothing to undo.\n"; continue; }
                auto undoOne = [this](){ Move last = moveHistory_.back(); moveHistory_.pop_back(); board_.undoMove(last); if(!pgnMoves_.empty()) pgnMoves_.pop_back(); };
                undoOne(); std::string msg = "Undid last move."; logAction(msg); std::cout<<msg<<"\n";
                if(vsComputer_ && !moveHistory_.empty()){ undoOne(); std::string msg2 = "Also undid computer move to restore your turn."; logAction(msg2); std::cout<<msg2<<"\n"; }
                printBoard(); continue; }
            if(cmd=="log"){ printActionLog(); continue; }
            // try as move
            if(makeMoveString(cmd)) { printBoard(); maybePlayComputerTurn(); continue; }
        } catch(const std::exception& ex){ std::cout << "Error: " << ex.what() << "\n"; }
    }
}

void Game::printActionLog() const {
    if(actionLog_.empty()){ std::cout << "No actions logged yet.\n"; return; }
    for(size_t i=0;i<actionLog_.size(); ++i){ std::cout << i+1 << ". " << actionLog_[i] << "\n"; }
}

std::string Game::pieceName(PieceType type) const {
    switch(type){
        case PieceType::King: return "king";
        case PieceType::Queen: return "queen";
        case PieceType::Rook: return "rook";
        case PieceType::Bishop: return "bishop";
        case PieceType::Knight: return "knight";
        case PieceType::Pawn: return "pawn";
        default: return "piece";
    }
}

std::string Game::describeMove(Color mover, PieceType movingType, const Move& m) const {
    std::ostringstream oss;
    std::string moverStr = mover==Color::White? "White" : "Black";
    std::string opponentStr = mover==Color::White? "Black" : "White";
    std::string movingName = pieceName(movingType);
    if(m.isCastle()){
        bool kingside = Board::squareToString(m.to)[0]=='g';
        oss << moverStr << " castles " << (kingside? "kingside" : "queenside");
        return oss.str();
    }
    if(m.isCapture()){
        std::string capturedName = pieceName(m.captured);
        oss << moverStr << ' ' << movingName << " captures " << opponentStr << ' ' << capturedName
            << " on " << Board::squareToString(m.to);
        if(m.isEnPassant()) oss << " (en passant)";
    } else {
        oss << moverStr << ' ' << movingName << " moves from " << Board::squareToString(m.from)
            << " to " << Board::squareToString(m.to);
    }
    if(m.isPromotion()){
        oss << ", promotes to " << pieceName(m.promotion);
    }
    return oss.str();
}

void Game::logAction(const std::string& entry){
    actionLog_.push_back(entry);
    std::ofstream ofs(logFilePath_, std::ios::app);
    if(ofs) ofs << entry << '\n';
}

void Game::resetLogFile(){ std::ofstream ofs(logFilePath_, std::ios::trunc); }

void Game::recordMove(const Move& executed, Color mover, PieceType movingType, const std::string& prefix){
    moveHistory_.push_back(executed);
    pgnMoves_.push_back(moveToAlgebraic(executed));
    std::string desc = describeMove(mover, movingType, executed);
    if(!prefix.empty()) desc = prefix + desc;
    logAction(desc);
    std::cout << desc << "\n";
}

void Game::maybePlayComputerTurn(){
    if(!vsComputer_) return;
    if(board_.sideToMove()!=computerSide_) return;

    std::vector<Move> legal;
    board_.generateLegalMoves(legal);
    if(legal.empty()){
        std::string msg = "Computer has no legal moves.";
        logAction(msg);
        std::cout << msg << "\n";
        return;
    }

    Move choice = chooseComputerMove(legal);
    Move backup = choice;
    const Piece* movingPiece = board_.pieceAt(choice.from);
    PieceType movingType = movingPiece? movingPiece->type(): PieceType::None;
    Color mover = computerSide_;
    board_.applyMove(choice);
    recordMove(backup, mover, movingType, "Computer: ");
    printBoard();
}

Move Game::chooseComputerMove(const std::vector<Move>& legal) const {
    for(const auto& move : legal){ if(move.isCapture()) return move; }
    for(const auto& move : legal){ if(move.isPromotion()) return move; }
    return legal.front();
}

void Game::configureMode(bool reconfigure){
    if(reconfigure) std::cout << "\n-- Configure new game --\n";
    std::string input;
    while(true){
        std::cout << "Play against the computer? (y/n): ";
        if(!std::getline(std::cin, input)) return;
        input = trim(input);
        if(input.empty()) continue;
        char c = static_cast<char>(std::tolower(static_cast<unsigned char>(input[0])));
        if(c=='y'){ vsComputer_ = true; break; }
        if(c=='n'){ vsComputer_ = false; break; }
        std::cout << "Please enter 'y' or 'n'.\n";
    }

    if(vsComputer_){
        while(true){
            std::cout << "Choose your color (w = White, b = Black): ";
            if(!std::getline(std::cin, input)) return;
            input = trim(input);
            if(input.empty()) continue;
            char c = static_cast<char>(std::tolower(static_cast<unsigned char>(input[0])));
            if(c=='w'){ humanSide_ = Color::White; break; }
            if(c=='b'){ humanSide_ = Color::Black; break; }
            std::cout << "Please enter 'w' or 'b'.\n";
        }
        computerSide_ = (humanSide_==Color::White)? Color::Black : Color::White;
        std::string msg = std::string("Mode: Human vs Computer. You play ") + (humanSide_==Color::White? "White." : "Black.");
        logAction(msg);
        std::cout << msg << "\n";
    } else {
        humanSide_ = Color::White;
        computerSide_ = Color::Black;
        std::string msg = "Mode: Human vs Human (two players).";
        logAction(msg);
        std::cout << msg << "\n";
    }
}

std::string Game::trim(const std::string& s){
    auto first = s.find_first_not_of(" \t\r\n");
    if(first==std::string::npos) return "";
    auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last-first+1);
}

} 