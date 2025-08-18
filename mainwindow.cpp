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
#include <QTableWidgetItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_board(new BoardView(15, this)),
    m_bag(new TileBag()),
    m_currentPlayer(0)
{
    // create two racks (players)
    m_racks[0] = new RackView(this);
    m_racks[1] = new RackView(this);

    setWindowTitle("Equatix — Two Player");
    auto *central = new QWidget(this);
    auto *v = new QVBoxLayout(central);

    QLabel *title = new QLabel("Equatix", this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 22px; font-weight: bold;");

    v->addWidget(title);
    v->addWidget(m_board, 1);

    // Racks row: Player 1 | Player 2
    auto *racksRow = new QHBoxLayout;
    {
        auto *p1Layout = new QVBoxLayout;
        p1Layout->addWidget(new QLabel("Player 1 Rack:"));
        p1Layout->addWidget(m_racks[0]);
        racksRow->addLayout(p1Layout);

        auto *spacer = new QWidget;
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        racksRow->addWidget(spacer);

        auto *p2Layout = new QVBoxLayout;
        p2Layout->addWidget(new QLabel("Player 2 Rack:"));
        p2Layout->addWidget(m_racks[1]);
        racksRow->addLayout(p2Layout);
    }
    v->addLayout(racksRow);

    // Toolbar actions
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
    statusBar()->showMessage("Player 1's turn. Drag tiles from your rack to the board to form valid equations.");

    // initial fill for both racks
    refillRack(0);
    refillRack(1);

    // ensure only the active player's rack is enabled
    enableRacksForCurrentPlayer();
}

void MainWindow::enableRacksForCurrentPlayer() {
    m_racks[0]->setEnabled(m_currentPlayer == 0);
    m_racks[1]->setEnabled(m_currentPlayer == 1);
}

void MainWindow::refillRack(int player) {
    if (player < 0 || player > 1) return;
    RackView *rack = m_racks[player];

    // Ensure each rack has exactly one '=' tile (if we want that invariant)
    if (!rack->hasEqualsTile()) {
        QChar eq = m_bag->drawEquals();
        if (!eq.isNull()) {
            rack->addTile(eq);
        }
    }

    // Fill other tiles up to 7 non-equals tiles
    while (rack->countNonEqualsTiles() < 7) {
        QChar ch = m_bag->drawOther();
        if (ch.isNull()) break;
        rack->addTile(ch);
    }
}

QVector<QVector<QChar>> MainWindow::boardSnapshot() const {
    int R = m_board->rowCount(), C = m_board->columnCount();
    QVector<QVector<QChar>> snap(R, QVector<QChar>(C));
    for (int r = 0; r < R; ++r) {
        for (int c = 0; c < C; ++c) {
            QTableWidgetItem *it = m_board->item(r, c);
            if (it && !it->text().isEmpty()) snap[r][c] = it->text().at(0);
            else snap[r][c] = QChar();
        }
    }
    return snap;
}

void MainWindow::endTurn() {
    // toggle player
    m_currentPlayer = 1 - m_currentPlayer;
    enableRacksForCurrentPlayer();
    statusBar()->showMessage(QString("Player %1's turn").arg(m_currentPlayer + 1), 2000);
}

void MainWindow::onValidate() {
    // ensure the active player actually placed tiles
    if (m_board->newTiles().isEmpty()) {
        QMessageBox::information(this, "Empty Turn", "You haven't placed any tiles.");
        return;
    }

    auto snap = boardSnapshot();
    QString why;
    if (!EquationValidator::validate(snap, m_board->newTiles(), why)) {
        QMessageBox::warning(this, "Invalid Turn", why);
        return;
    }

    // lock tiles and refill only the current player's rack
    m_board->lockNewTiles();
    refillRack(m_currentPlayer);

    // end turn: switch to other player
    endTurn();
}

void MainWindow::onUndo() {
    // only the current player may undo their new placements during their turn
    QList<QChar> returned;
    m_board->rollbackNewTiles(returned);
    // give tiles back to current player's rack
    for (QChar c : returned) {
        m_racks[m_currentPlayer]->addTile(c);
    }
    statusBar()->showMessage("Undid placements", 1500);
}

void MainWindow::onSwap() {
    // Swap operates on current player's rack
    RackView *rack = m_racks[m_currentPlayer];

    SwapDialog dlg(rack->nonEqualsTiles(), this);
    if (dlg.exec() == QDialog::Accepted) {
        QList<QChar> tilesToSwap = dlg.getSelectedTiles();
        if (tilesToSwap.isEmpty()) {
            return; // nothing selected
        }

        int swapCount = tilesToSwap.count();

        // Ensure bag has enough tiles
        if (m_bag->otherTilesCount() < swapCount) {
            QMessageBox::warning(this, "Cannot Swap", "There are not enough tiles left in the bag to perform this swap.");
            return;
        }

        // Remove selected tiles from player's rack
        rack->removeTiles(tilesToSwap);

        // Return them to bag
        m_bag->returnTiles(tilesToSwap);

        // Draw replacements and add them to player's rack
        for (int i = 0; i < swapCount; ++i) {
            QChar newTile = m_bag->drawOther();
            if (!newTile.isNull()) rack->addTile(newTile);
        }

        // end player's turn after swapping
        endTurn();
        statusBar()->showMessage(QString("Player %1 swapped %2 tile(s).").arg(m_currentPlayer == 0 ? 2 : 1).arg(swapCount), 2000);
    }
}
