#ifndef TILEBAG_H
#define TILEBAG_H

#include <QVector>
#include <QChar>

class TileBag {
public:
    TileBag();
    bool empty() const { return m_idx >= m_tiles.size(); }
    QChar draw(); // returns null QChar if empty
private:
    QVector<QChar> m_tiles;
    int m_idx = 0;
};

#endif // TILEBAG_H

