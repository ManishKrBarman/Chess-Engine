#pragma once
/**
 * Game controller handling user input, game loop, PGN log, saving/loading.
 */
#include <vector>
#include <string>
#include <fstream>
#include "Board.h"
#include "Move.h"

namespace chess {

class Game {
public:
    Game();

    void newGame();
    void printBoard() const;
    void loop(); // main CLI loop

    void save(const std::string& filename) const; // save FEN
    void load(const std::string& filename);

    void exportPGN(const std::string& filename) const;

    void printActionLog() const;

private:
    Board board_;
    std::vector<Move> moveHistory_;
    std::vector<std::string> pgnMoves_;
    std::vector<std::string> actionLog_;
    std::string logFilePath_;
    bool vsComputer_ { true };
    Color computerSide_ { Color::Black };
    Color humanSide_ { Color::White };

    bool makeMoveString(const std::string& moveStr, bool record = true);
    std::string moveToAlgebraic(const Move& m) const; // simplistic
    std::string describeMove(Color mover, PieceType movingType, const Move& m) const;
    std::string pieceName(PieceType type) const;
    void logAction(const std::string& entry);
    void resetLogFile();
    void recordMove(const Move& executed, Color mover, PieceType movingType, const std::string& prefix = "");
    void maybePlayComputerTurn();
    Move chooseComputerMove(const std::vector<Move>& legal) const;
    void configureMode(bool reconfigure);
    static std::string trim(const std::string& s);
};

} // namespace chess
