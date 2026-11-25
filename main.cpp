// Stonevault - a tiny dungeon crawler in C++.
//
// A 12x8 vault is generated for each run: walls, gold, monsters that
// chase you, and an exit. Bump into a monster to fight it. Reach the
// exit with as much gold as you can carry - if your HP runs out, the
// vault keeps you.
// Commands: w/a/s/d move, q quit. Usage: ./stonevault [--seed N]
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

const int W = 12;
const int H = 8;

unsigned long rng_state = 1;
unsigned rng_next() {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return static_cast<unsigned>(rng_state & 0x7fffffffUL);
}

struct Monster { int x, y, hp; bool alive = true; };

} // namespace

int main(int argc, char **argv) {
    unsigned long seed = 20261125UL;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--seed" && i + 1 < argc) seed = std::strtoul(argv[++i], nullptr, 10);
    }
    rng_state = seed ? seed : 1;

    // Carve a room-and-pillars vault: border walls plus random pillars.
    std::vector<std::string> map(H, std::string(W, '.'));
    for (int x = 0; x < W; x++) { map[0][x] = '#'; map[H - 1][x] = '#'; }
    for (int y = 0; y < H; y++) { map[y][0] = '#'; map[y][W - 1] = '#'; }
    for (int i = 0; i < 8; i++) {
        int x = 1 + static_cast<int>(rng_next() % (W - 2));
        int y = 1 + static_cast<int>(rng_next() % (H - 2));
        if ((x == 1 && y == 1) || (x == W - 2 && y == H - 2)) continue;
        map[y][x] = '#';
    }

    int px = 1, py = 1;
    int ex = W - 2, ey = H - 2;
    map[ey][ex] = 'E';

    int gold_spots = 5;
    for (int i = 0; i < gold_spots; i++) {
        int x, y;
        do {
            x = 1 + static_cast<int>(rng_next() % (W - 2));
            y = 1 + static_cast<int>(rng_next() % (H - 2));
        } while (map[y][x] != '.' || (x == px && y == py));
        map[y][x] = 'g';
    }

    std::vector<Monster> monsters;
    for (int i = 0; i < 3; i++) {
        int x, y;
        do {
            x = 1 + static_cast<int>(rng_next() % (W - 2));
            y = 1 + static_cast<int>(rng_next() % (H - 2));
        } while (map[y][x] != '.' || (x == px && y == py) ||
                 (std::abs(x - px) + std::abs(y - py)) < 4);
        monsters.push_back({x, y, 2, true});
    }

    int hp = 10, gold = 0;
    std::cout << "=== STONEVAULT ===\n"
              << "Find the exit (E). Gold (g) is worth 10. Monsters (M) chase you;\n"
              << "bump them to fight. You have 10 HP. Commands: w/a/s/d, q.\n\n";

    std::string line;
    bool done = false, won = false;
    while (!done) {
        // Draw
        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                char c = map[y][x];
                for (const auto &m : monsters)
                    if (m.alive && m.x == x && m.y == y) c = 'M';
                if (x == px && y == py) c = '@';
                std::cout << c;
            }
            std::cout << '\n';
        }
        std::cout << "HP " << hp << "  Gold " << gold << "\n> " << std::flush;
        if (!std::getline(std::cin, line)) { std::cout << "\n"; break; }
        if (line.empty()) continue;
        char cmd = line[0];
        if (cmd == 'q') break;
        int dx = 0, dy = 0;
        if (cmd == 'w') dy = -1;
        else if (cmd == 's') dy = 1;
        else if (cmd == 'a') dx = -1;
        else if (cmd == 'd') dx = 1;
        else continue;

        int nx = px + dx, ny = py + dy;
        // Attack?
        bool attacked = false;
        for (auto &m : monsters) {
            if (m.alive && m.x == nx && m.y == ny) {
                m.hp -= 1;
                attacked = true;
                if (m.hp <= 0) {
                    m.alive = false;
                    gold += 15;
                    std::cout << "Monster slain! +15 gold.\n";
                } else {
                    hp -= 1;
                    std::cout << "You hit the monster; it claws back (-1 HP).\n";
                }
                break;
            }
        }
        if (!attacked && map[ny][nx] != '#') {
            px = nx; py = ny;
            if (map[py][px] == 'g') {
                map[py][px] = '.';
                gold += 10;
                std::cout << "Picked up gold (+10).\n";
            }
            if (px == ex && py == ey) { won = true; done = true; }
        }
        if (hp <= 0) { done = true; break; }
        if (done) break;

        // Monsters chase: step toward the player if not adjacent-blocked.
        for (auto &m : monsters) {
            if (!m.alive) continue;
            int mdx = (px > m.x) ? 1 : (px < m.x ? -1 : 0);
            int mdy = (py > m.y) ? 1 : (py < m.y ? -1 : 0);
            int tx = m.x, ty = m.y;
            if (std::abs(px - m.x) > std::abs(py - m.y)) tx += mdx; else ty += mdy;
            if (tx == px && ty == py) {
                hp -= 1;
                std::cout << "The monster bites you (-1 HP).\n";
                if (hp <= 0) { done = true; }
                continue;
            }
            bool blocked = map[ty][tx] == '#' || map[ty][tx] == 'E';
            for (const auto &o : monsters)
                if (o.alive && &o != &m && o.x == tx && o.y == ty) blocked = true;
            if (!blocked) { m.x = tx; m.y = ty; }
        }
        if (hp <= 0) done = true;
        std::cout << '\n';
    }

    std::cout << "\n=== GAME OVER ===\n";
    if (won) {
        int total = gold + hp * 5;
        std::cout << "You escaped the Stonevault with " << gold << " gold!\n"
                  << "Final score: " << total << " (gold + 5 per HP left)\n";
    } else if (hp <= 0) {
        std::cout << "The vault keeps you. Gold gathered: " << gold << "\nFinal score: " << gold << "\n";
    } else {
        std::cout << "You leave the vault behind. Gold gathered: " << gold << "\nFinal score: " << gold << "\n";
    }
    return 0;
}
