#include "BoardView.h"
#include <QHeaderView>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>

static const char* kMimeType = "application/x-equatix-tile";

BoardView::BoardView(int n, QWidget *parent)
    : QTableWidget(n, n, parent), N(n)
{
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DropOnly);
    setSelectionMode(QAbstractItemView::NoSelection);
    horizontalHeader()->setVisible(false);
    verticalHeader()->setVisible(false);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    // prevent scrollbars from appearing (they would occupy the margins)

    // remove the frame if you want a clean centered look
    setFrameStyle(QFrame::NoFrame);
    for (int r=0;r<N;++r) {
        setRowHeight(r, 40);
        setColumnWidth(r, 40);
        for (int c=0;c<N;++c) {
            QTableWidgetItem *it = new QTableWidgetItem("");
            it->setTextAlignment(Qt::AlignCenter);
            it->setData(Qt::UserRole+1, false); // locked?
            setItem(r,c,it);
        }
    }
    setStyleSheet("QTableWidget::item{ font-weight: 700; font-size: 16px; }");


}

void BoardView::resizeEvent(QResizeEvent* e) {
    QTableWidget::resizeEvent(e);

    // compute a square cell size
    int s = qMax(1, qMin(width() / N, height() / N));

    // apply same size to every row/col so cells stay square
    for (int i = 0; i < N; ++i) {
        setRowHeight(i, s);
        setColumnWidth(i, s);
    }

    // total grid size
    int gridW = s * N;
    int gridH = s * N;

    // margins to center the viewport (non-negative)
    int left  = qMax(0, (width()  - gridW) / 2);
    int top   = qMax(0, (height() - gridH) / 2);

    // move the viewport inward so the grid appears centered
    setViewportMargins(left, top, left, top);

    // force a repaint if needed
    viewport()->update();
}


void BoardView::dragEnterEvent(QDragEnterEvent* e) {
    if (e->mimeData()->hasFormat(kMimeType)) e->acceptProposedAction();
}

void BoardView::dragMoveEvent(QDragMoveEvent* e) {
    e->acceptProposedAction();
}

bool BoardView::isAllowed(QChar ch) const {
    return ch.isDigit() || QString("+-*/=").contains(ch);
}

void BoardView::dropEvent(QDropEvent* e) {
    QPoint pos = e->position().toPoint();
    int r = rowAt(pos.y());
    int c = columnAt(pos.x());
    if (r < 0 || c < 0) return;
    QTableWidgetItem* it = item(r,c);
    if (!it) return;
    if (it->data(Qt::UserRole+1).toBool()) return; // locked
    if (!it->text().isEmpty()) return; // occupied

    if (!e->mimeData()->hasFormat(kMimeType)) return;
    QByteArray ba = e->mimeData()->data(kMimeType);
    if (ba.isEmpty()) return;
    QChar ch(ba.at(0));
    if (!isAllowed(ch)) return;

    it->setText(QString(ch));
    it->setBackground(QBrush(QColor(255,248,200)));
    m_newThisTurn.insert({r,c});
    e->acceptProposedAction();
}

void BoardView::lockNewTiles() {
    for (auto rc : m_newThisTurn) {
        QTableWidgetItem* it = item(rc.first, rc.second);
        if (it) {
            it->setData(Qt::UserRole+1, true);
            it->setBackground(QBrush(QColor(235,255,235)));
        }
    }
    m_newThisTurn.clear();
}

void BoardView::rollbackNewTiles(QList<QChar> &returned) {
    for (auto rc : m_newThisTurn) {
        QTableWidgetItem* it = item(rc.first, rc.second);
        if (it && !it->text().isEmpty()) {
            returned.append(it->text().at(0));
            it->setText("");
            it->setBackground(QBrush());
        }
    }
    m_newThisTurn.clear();
}

void BoardView::clearNewMarks() {
    for (int r=0;r<N;++r) for (int c=0;c<N;++c) {
            QTableWidgetItem* it = item(r,c);
            if (it && !it->data(Qt::UserRole+1).toBool()) {
                it->setBackground(QBrush());
            }
        }
    m_newThisTurn.clear();
}

