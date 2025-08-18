#ifndef BOARDVIEW_H
#define BOARDVIEW_H

#include <QWidget>
#include <QTableWidget>
#include <QSet>
#include <QPair>

struct Placement { int row; int col; QChar ch; };

enum MultiplierType {
    None,
    DoublePiece,
    TriplePiece,
    DoubleEquation,
    TripleEquation
};

class BoardView : public QTableWidget {
    Q_OBJECT
public:
    explicit BoardView(int n=15, QWidget *parent=nullptr);

    const QSet<QPair<int,int>>& newTiles() const { return m_newThisTurn; }
    void lockNewTiles();
    void rollbackNewTiles(QList<QChar> &returned); // returns chars to rack
    void clearNewMarks();

protected:
    void dragEnterEvent(QDragEnterEvent* e) override;
    void dragMoveEvent(QDragMoveEvent* e) override;
    void dropEvent(QDropEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

private:
    bool isAllowed(QChar ch) const;
    void initializeMultipliers();

    int N;
    QSet<QPair<int,int>> m_newThisTurn;
    QMap<QPair<int,int>, MultiplierType> m_multiplierMap;
};

#endif // BOARDVIEW_H
