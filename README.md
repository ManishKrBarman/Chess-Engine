# ChessEngine

Minimal two-player console chess written in modern C++17. The code favors clarity over raw engine strength.

## Features

- Object-oriented `Piece` hierarchy (king/queen/rook/bishop/knight/pawn) with polymorphic `generateMoves` implementations.
- `Board` stores an 8×8 grid of smart pointers, castling rights, en-passant target, and offers pseudo/legal move generation (castling, en-passant, promotions with optional suffix).
- `Game` controller powers a compact CLI: print the board, enter coordinate moves (e2e4), undo, save/load FEN, export PGN, view a descriptive move log, and choose whether to face another human or the built-in computer (with the option to play White or Black).
- `Move` struct keeps everything needed for undo (captured type, previous castling flags, en-passant square, half-move clock placeholder).
- Exceptions make invalid commands or file errors explicit.

> Simplifications: no 50-move or repetition detection, castling legality assumes attack checks handled elsewhere, promotions default to queen unless you append `q/r/b/n`.

## Build

From the `ChessEngine/` directory run:

```bash
g++ -std=c++17 -O2 -Iinclude src/*.cpp -o chess
```

or simply:

```bash
make
```

Both generate `./chess` in the same folder.

## Run

```bash
./chess
```

When the program starts (and whenever you type `new`), it will ask:

1. `Play against the computer? (y/n)` — choose `n` for hot-seat human vs human play.
2. If you chose `y`, answer `w` or `b` to pick your color. The computer takes the opposite color and automatically responds after each of your moves; if you pick Black, the computer makes the first move immediately.

Use `new` any time you want to re-select the opponent or color.

Example session:

```
e2e4
... (engine prints "White pawn moves from e2 to e4")
e7e5
log
save opening.fen
pgn game.pgn
undo
quit
```

## Commands

| Command     | Description                                                     |
|-------------|-----------------------------------------------------------------|
| `e2e4`      | Make a move; add `q/r/b/n` for promotion piece.                 |
| `save FILE` | Save the current position (FEN) to `FILE`.                      |
| `load FILE` | Load a FEN from `FILE`.                                         |
| `pgn FILE`  | Export the played moves to a lightweight PGN file.              |
| `undo`      | Undo your move *and* the computer's reply so it's your turn.    |
| `log`       | Print the natural-language action log for this game.            |
| `new`       | Reset to the initial position (clears history/log and re-prompts for opponent/color). |
| `board`     | Reprint the ASCII board.                                        |
| `help`      | Show the command summary.                                       |
| `quit`      | Exit the program.                                               |

Every move is echoed with a readable description (e.g., `Black bishop captures White knight on g4`). These entries are also appended to `game_log.txt` beside the executable so you can review them later. When you play a move, the computer immediately responds with a simple heuristic (captures > promotions > first legal move) and its action is logged with the `Computer:` prefix.

## Notes

- PGN export labels the players `Player1`/`Player2` and leaves the result as `*`.
- Castling/en-passant legality is basic; treat this engine as a teaching baseline you can extend.
- Loading a FEN (`load FILE`) also asks which mode/color you want, so you can resume positions as either side.

Have fun hacking on console chess!
