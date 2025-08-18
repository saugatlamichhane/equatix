#include "TileBag.h"
#include <algorithm>
#include <random>
#include <chrono>

TileBag::TileBag() {
    auto add = [&](QChar ch, int count){
        if (ch == '=') {
            for (int i=0; i<count; ++i) m_equalsTiles.append(ch);
        } else {
            for (int i=0; i<count; ++i) m_otherTiles.append(ch);
        }
    };

    // Distribution: digits heavy, ops fewer, equals some
    for (int d='0'; d<='9'; ++d) add(QChar(d), 6); // 60 digits
    add('+', 10);
    add('-', 10);
    add('*', 8);
    add('/', 8);
    add('=', 12); // Equals tiles are now managed separately

    shuffleOthers();
}

void TileBag::shuffleOthers() {
    unsigned seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 rng(seed);
    std::shuffle(m_otherTiles.begin(), m_otherTiles.end(), rng);
    m_otherIdx = 0; // Reset index after shuffle
}

QChar TileBag::drawEquals() {
    if (m_equalsIdx >= m_equalsTiles.size()) return QChar();
    return m_equalsTiles[m_equalsIdx++];
}

QChar TileBag::drawOther() {
    if (otherTilesEmpty()) return QChar();
    return m_otherTiles[m_otherIdx++];
}

bool TileBag::otherTilesEmpty() const {
    return m_otherIdx >= m_otherTiles.size();
}

int TileBag::otherTilesCount() const {
    return m_otherTiles.size() - m_otherIdx;
}

void TileBag::returnTiles(const QList<QChar>& chars) {
    // This is a simple implementation: add tiles back to the end of the vector.
    // A more robust implementation might re-insert them at the current index.
    for (QChar ch : chars) {
        if (ch == '=') {
            // This case shouldn't happen with the swap logic, but is safe to have
            if (m_equalsIdx > 0) m_equalsIdx--;
        } else {
            if (m_otherIdx > 0) {
                m_otherIdx--;
                m_otherTiles[m_otherIdx] = ch; // Put it back in the available pool
            }
        }
    }
    // Re-shuffle the bag to mix the returned tiles in
    shuffleOthers();
}
