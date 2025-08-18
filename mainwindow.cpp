#include "mainwindow.h"
#include "BoardView.h"
#include "RackView.h"
#include "TileBag.h"
#include "EquationValidator.h"
#include "SwapDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QMessageBox>
#include <QStatusBar>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_board(new BoardView(15, this)),
    m_rack(new RackView(this)),
    m_bag(new TileBag())
{
    setWindowTitle("Equatix");
    auto *central = new QWidget(this);
    auto *v = new QVBoxLayout(central);

    QLabel *title = new QLabel("Equatix", this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 22px; font-weight: bold;");

    v->addWidget(title);
    v->addWidget(m_board, 1);

    auto *rackRow = new QHBoxLayout;
    rackRow->addWidget(new QLabel("Rack:"));
    rackRow->addWidget(m_rack, 1);
    v->addLayout(rackRow);

    auto *toolbar = addToolBar("Actions");
    QAction *validate = new QAction("Validate Turn", this);
    QAction *undo = new QAction("Undo", this);
    QAction *swap = new QAction("Swap Tiles", this);
    toolbar->addAction(validate);
    toolbar->addAction(undo);
    toolbar->addAction(swap);

    connect(validate, &QAction::triggered, this, &MainWindow::onValidate);
    connect(undo, &QAction::triggered, this, &MainWindow::onUndo);
    connect(swap, &QAction::triggered, this, &MainWindow::onSwap);

    setCentralWidget(central);
    statusBar()->showMessage("Drag tiles to the board to form valid equations.");

    refillRack();
}

void MainWindow::refillRack() {
    if (!m_rack->hasEqualsTile()) {
        QChar eq = m_bag->drawEquals();
        if (!eq.isNull()) {
            m_rack->addTile(eq);
        }
    }
    while (m_rack->countNonEqualsTiles() < 7) {
        QChar ch = m_bag->drawOther();
        if (ch.isNull()) break;
        m_rack->addTile(ch);
    }
}

void MainWindow::onValidate() {
    if (m_board->newTiles().isEmpty()){
        QMessageBox::information(this, "Empty Turn", "You haven't placed any tiles.");
        return;
    }
    auto snap = boardSnapshot();
    QString why;
    if (!EquationValidator::validate(snap, m_board->newTiles(), why)) {
        QMessageBox::warning(this, "Invalid Turn", why);
        return;
    }
    m_board->lockNewTiles();
    refillRack();
    statusBar()->showMessage("Turn accepted ✔", 2000);
}

void MainWindow::onUndo() {
    QList<QChar> returned;
    m_board->rollbackNewTiles(returned);
    for (QChar c : returned) {
        m_rack->addTile(c);
    }
    statusBar()->showMessage("Undid placements", 1500);
}

// ### MODIFIED SECTION ###
void MainWindow::onSwap() {
    SwapDialog dlg(m_rack->nonEqualsTiles(), this);
    if (dlg.exec() == QDialog::Accepted) {
        QList<QChar> tilesToSwap = dlg.getSelectedTiles();
        if (tilesToSwap.isEmpty()) {
            return; // User clicked OK but selected nothing
        }

        int swapCount = tilesToSwap.count();

        // **Improved Check**: Ensure bag has enough tiles for a 1-for-1 swap.
        if (m_bag->otherTilesCount() < swapCount) {
            QMessageBox::warning(this, "Cannot Swap", "There are not enough tiles left in the bag to perform this swap.");
            return;
        }

        // 1. Remove selected tiles from rack
        m_rack->removeTiles(tilesToSwap);

        // 2. Return tiles to the bag
        m_bag->returnTiles(tilesToSwap);

        // 3. **Direct Replacement**: Draw the exact number of new tiles.
        for (int i = 0; i < swapCount; ++i) {
            QChar newTile = m_bag->drawOther();
            if (!newTile.isNull()) {
                m_rack->addTile(newTile);
            }
        }

        statusBar()->showMessage(QString("Swapped %1 tile(s).").arg(swapCount), 2000);
    }
}
// ### END MODIFIED SECTION ###


QVector<QVector<QChar>> MainWindow::boardSnapshot() const {
    int R = m_board->rowCount(), C = m_board->columnCount();
    QVector<QVector<QChar>> snap(R, QVector<QChar>(C));
    for (int r=0; r<R; ++r) for (int c=0; c<C; ++c) {
            QTableWidgetItem *it = m_board->item(r,c);
            if (it && !it->text().isEmpty()) snap[r][c] = it->text().at(0);
            else snap[r][c] = QChar();
        }
    return snap;
}
