#include "BoardView.h"
#include <QHeaderView>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QBrush>
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
    setFrameStyle(QFrame::NoFrame);

    initializeMultipliers();

    for (int r=0;r<N;++r) {
        setRowHeight(r, 40);
        setColumnWidth(r, 40);
        for (int c=0;c<N;++c) {
            QTableWidgetItem *it = new QTableWidgetItem("");
            it->setTextAlignment(Qt::AlignCenter);
            it->setData(Qt::UserRole+1, false); // locked?

            MultiplierType mt = m_multiplierMap.value({r,c}, None);
            it->setData(Qt::UserRole+2, mt);

            // set base color for multiplier
            switch(mt) {
            case DoublePiece: it->setBackground(QColor(173,216,230)); break; // light blue
            case TriplePiece: it->setBackground(QColor(0,191,255)); break;   // deep sky blue
            case DoubleEquation: it->setBackground(QColor(255,182,193)); break; // light pink
            case TripleEquation: it->setBackground(QColor(255,69,0)); break; // orange red
            default: break;
            }

            setItem(r,c,it);
        }
    }

    setStyleSheet("QTableWidget::item{ font-weight: 700; font-size: 16px; }");
}

void BoardView::initializeMultipliers() {
    // Triple Equation (like triple word score)
    QList<QPair<int,int>> tripleEq = {{0,0},{0,7},{0,14},{7,0},{7,14},{14,0},{14,7},{14,14}};
    for (auto p : tripleEq) m_multiplierMap[p] = TripleEquation;

    // Double Equation
    QList<QPair<int,int>> doubleEq = {
        {1,1},{2,2},{3,3},{4,4},{7,7},{10,10},{11,11},{12,12},{13,13},
        {1,13},{2,12},{3,11},{4,10},{10,4},{11,3},{12,2},{13,1}
    };
    for (auto p : doubleEq) m_multiplierMap[p] = DoubleEquation;

    // Triple Piece
    QList<QPair<int,int>> triplePc = {
        {1,5},{1,9},{5,1},{5,5},{5,9},{5,13},{9,1},{9,5},{9,9},{9,13},{13,5},{13,9}
    };
    for (auto p : triplePc) m_multiplierMap[p] = TriplePiece;

    // Double Piece
    QList<QPair<int,int>> doublePc = {
        {0,3},{0,11},{2,6},{2,8},{3,0},{3,7},{3,14},
        {6,2},{6,6},{6,8},{6,12},{7,3},{7,11},
        {8,2},{8,6},{8,8},{8,12},{11,0},{11,7},{11,14},
        {12,6},{12,8},{14,3},{14,11}
    };
    for (auto p : doublePc) m_multiplierMap[p] = DoublePiece;
}

void BoardView::resizeEvent(QResizeEvent* e) {
    QTableWidget::resizeEvent(e);
    int s = qMax(1, qMin(width() / N, height() / N));
    for (int i = 0; i < N; ++i) { setRowHeight(i, s); setColumnWidth(i, s); }
    int gridW = s * N, gridH = s * N;
    int left  = qMax(0, (width()  - gridW) / 2);
    int top   = qMax(0, (height() - gridH) / 2);
    setViewportMargins(left, top, left, top);
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
    it->setBackground(QColor(255,248,200)); // temporary highlight
    m_newThisTurn.insert({r,c});
    e->acceptProposedAction();
}

void BoardView::lockNewTiles() {
    for (auto rc : m_newThisTurn) {
        QTableWidgetItem* it = item(rc.first, rc.second);
        if (it) {
            it->setData(Qt::UserRole+1, true);
            it->setBackground(QColor(235,255,235)); // locked
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

            // restore multiplier color
            MultiplierType mt = static_cast<MultiplierType>(it->data(Qt::UserRole+2).toInt());
            switch(mt) {
            case DoublePiece: it->setBackground(QColor(173,216,230)); break;
            case TriplePiece: it->setBackground(QColor(0,191,255)); break;
            case DoubleEquation: it->setBackground(QColor(255,182,193)); break;
            case TripleEquation: it->setBackground(QColor(255,69,0)); break;
            default: it->setBackground(QBrush()); break;
            }
        }
    }
    m_newThisTurn.clear();
}

void BoardView::clearNewMarks() {
    for (int r=0;r<N;++r) for (int c=0;c<N;++c) {
            QTableWidgetItem* it = item(r,c);
            if (it && !it->data(Qt::UserRole+1).toBool()) {
                MultiplierType mt = static_cast<MultiplierType>(it->data(Qt::UserRole+2).toInt());
                switch(mt) {
                case DoublePiece: it->setBackground(QColor(173,216,230)); break;
                case TriplePiece: it->setBackground(QColor(0,191,255)); break;
                case DoubleEquation: it->setBackground(QColor(255,182,193)); break;
                case TripleEquation: it->setBackground(QColor(255,69,0)); break;
                default: it->setBackground(QBrush()); break;
                }
            }
        }
    m_newThisTurn.clear();
}
