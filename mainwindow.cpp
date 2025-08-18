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

    // Racks row: Player 1 | Player 2 + scores
    auto *racksRow = new QHBoxLayout;
    {
        auto *p1Layout = new QVBoxLayout;
        QLabel *p1label = new QLabel("Player 1 Rack:");
        p1label->setAlignment(Qt::AlignCenter);
        p1Layout->addWidget(p1label);
        p1Layout->addWidget(m_racks[0]);
        racksRow->addLayout(p1Layout);

        auto *spacer = new QWidget;
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        racksRow->addWidget(spacer);

        auto *p2Layout = new QVBoxLayout;
        QLabel *p2label = new QLabel("Player 2 Rack:");
        p2label->setAlignment(Qt::AlignCenter);
        p2Layout->addWidget(p2label);
        p2Layout->addWidget(m_racks[1]);
        racksRow->addLayout(p2Layout);
    }
    v->addLayout(racksRow);

    // Score bar (below racks)
    auto *scoreRow = new QHBoxLayout;
    m_scoreLabels[0] = new QLabel("Player 1: 0");
    m_scoreLabels[1] = new QLabel("Player 2: 0");
    scoreRow->addWidget(m_scoreLabels[0]);
    scoreRow->addStretch();
    scoreRow->addWidget(m_scoreLabels[1]);
    v->addLayout(scoreRow);

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

static int leftmost(const QVector<QVector<QChar>>& b, int r, int c) {
    while (c-1 >= 0 && !b[r][c-1].isNull()) --c;
    return c;
}
static int topmost(const QVector<QVector<QChar>>& b, int r, int c) {
    while (r-1 >= 0 && !b[r-1][c].isNull()) --r;
    return r;
}

// Tile base scoring rules:
// digits '1'-'9' => numeric value
// '0' => 1 point
// operators '+-*/' => 2 points
// '=' => 0 points
static int baseTileScore(QChar ch) {
    if (ch.isDigit()) {
        if (ch == '0') return 1;
        return ch.digitValue();
    }
    if (QString("+-*/").contains(ch)) return 2;
    return 0; // '=' or blank
}

int MainWindow::computeScoreForTurn(const QVector<QVector<QChar>>& snap, const QSet<QPair<int,int>>& newTiles) {
    // Calculate score for all distinct equations (horizontal and vertical) that are formed/affected by new tiles.
    // Multipliers are applied only if multiplier not previously used (we check m_board->multiplierUsedAt).
    int rows = snap.size();
    int cols = snap[0].size();

    QSet<QString> countedRuns;
    int total = 0;

    for (auto rc : newTiles) {
        int r = rc.first, c = rc.second;

        // Horizontal run
        int startC = leftmost(snap, r, c);
        int j = startC;
        QString runH;
        while (j < cols && !snap[r][j].isNull()) { runH.append(snap[r][j]); ++j; }
        if (runH.size() >= 2 && runH.contains('=')) {
            QString key = QString("H%1:%2").arg(r).arg(runH);
            if (!countedRuns.contains(key)) {
                countedRuns.insert(key);

                // compute score for this run
                long long runScore = 0;
                long long equationMultiplier = 1;
                for (int cc = startC; cc < j; ++cc) {
                    QChar ch = snap[r][cc];
                    int tileScore = baseTileScore(ch);
                    // piece multiplier applies only if multiplier there and not used yet
                    MultiplierType mt = m_board->multiplierAt(r, cc);
                    bool used = m_board->multiplierUsedAt(r, cc);
                    if (!used) {
                        if (mt == DoublePiece) tileScore *= 2;
                        else if (mt == TriplePiece) tileScore *= 3;
                        else if (mt == DoubleEquation) equationMultiplier *= 2;
                        else if (mt == TripleEquation) equationMultiplier *= 3;
                    }
                    runScore += tileScore;
                }
                runScore *= equationMultiplier;
                total += int(runScore);
            }
        }

        // Vertical run
        int startR = topmost(snap, r, c);
        int i = startR;
        QString runV;
        while (i < rows && !snap[i][c].isNull()) { runV.append(snap[i][c]); ++i; }
        if (runV.size() >= 2 && runV.contains('=')) {
            QString key = QString("V%1:%2").arg(c).arg(runV);
            if (!countedRuns.contains(key)) {
                countedRuns.insert(key);

                long long runScore = 0;
                long long equationMultiplier = 1;
                for (int rr = startR; rr < i; ++rr) {
                    QChar ch = snap[rr][c];
                    int tileScore = baseTileScore(ch);
                    MultiplierType mt = m_board->multiplierAt(rr, c);
                    bool used = m_board->multiplierUsedAt(rr, c);
                    if (!used) {
                        if (mt == DoublePiece) tileScore *= 2;
                        else if (mt == TriplePiece) tileScore *= 3;
                        else if (mt == DoubleEquation) equationMultiplier *= 2;
                        else if (mt == TripleEquation) equationMultiplier *= 3;
                    }
                    runScore += tileScore;
                }
                runScore *= equationMultiplier;
                total += int(runScore);
            }
        }
    }

    return total;
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

    // compute score for this turn (before consuming multipliers)
    int points = computeScoreForTurn(snap, m_board->newTiles());
    m_scores[m_currentPlayer] += points;
    m_scoreLabels[m_currentPlayer]->setText(QString("Player %1: %2").arg(m_currentPlayer + 1).arg(m_scores[m_currentPlayer]));

    // lock tiles and consume multipliers for newly covered squares
    m_board->lockNewTiles();

    // refill only the current player's rack
    refillRack(m_currentPlayer);

    // end turn: switch to other player
    endTurn();

    statusBar()->showMessage(QString("Player %1 scored %2 points.").arg(m_currentPlayer == 0 ? 2 : 1).arg(points), 3000);
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
