#include "TileBag.h"
#include <algorithm>
#include <random>
#include <chrono>

TileBag::TileBag() {
    auto add = [&](QChar ch, int count){
        for (int i=0;i<count;++i) m_tiles.append(ch);
    };
    // distribution: digits heavy, ops fewer, equals some
    for (int d='0'; d<='9'; ++d) add(QChar(d), 6); // 60 digits
    add('+', 10);
    add('-', 10);
    add('*', 8);
    add('/', 8);
    add('=', 12);

    std::mt19937 rng((unsigned)std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::shuffle(m_tiles.begin(), m_tiles.end(), rng);
}

QChar TileBag::draw() {
    if (empty()) return QChar();
    return m_tiles[m_idx++];
}

