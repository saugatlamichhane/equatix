#ifndef BOARDVIEW_H
#define BOARDVIEW_H

#include <QWidget>
#include <QTableWidget>
#include <QSet>
#include <QPair>

struct Placement { int row; int col; QChar ch; };

class BoardView : public QTableWidget {
    Q_OBJECT
public:
    explicit BoardView(int n=11, QWidget *parent=nullptr);

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
    int N;
    QSet<QPair<int,int>> m_newThisTurn;
};

#endif // BOARDVIEW_H
