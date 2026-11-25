# Stonevault (C++)

A tiny dungeon crawler for the terminal, written in C++ (C++17, standard
library only).

Each run generates a 12x8 vault: border walls, random pillars, five gold
piles, three monsters with chase AI, and an exit in the far corner. Bump
into a monster to fight it — it claws back while it lives, and pays out
gold when slain. Reach the exit to escape; your score is your gold plus
5 per HP left. Run out of HP and the vault keeps you.

## Build

```sh
g++ -std=c++17 -O2 -Wall -Wextra -o stonevault main.cpp
```

## Play

```sh
./stonevault             # random vault
./stonevault --seed 42   # reproducible vault (fixed layout)
```

Commands: `w` / `a` / `s` / `d` to move, `q` to quit.

## Tested envelope

- Built clean with `g++ -std=c++17 -O2 -Wall -Wextra` (GCC 13), no warnings.
- Verified end to end with scripted stdin playthroughs on `--seed 42`:
  a full escape run reaching the exit and the game-over score screen,
  and a quit path.
