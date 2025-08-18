#ifndef TILEBAG_H
#define TILEBAG_H

#include <QVector>
#include <QChar>
#include <QList>

class TileBag {
public:
    TileBag();
    bool otherTilesEmpty() const;
    int otherTilesCount() const;

    QChar drawEquals(); // Draws from the equals pile
    QChar drawOther();  // Draws from the numbers/operators pile

    void returnTiles(const QList<QChar>& chars); // For swapping

private:
    void shuffleOthers();

    QVector<QChar> m_equalsTiles;
    QVector<QChar> m_otherTiles;
    int m_equalsIdx = 0;
    int m_otherIdx = 0;
};

#endif // TILEBAG_H
