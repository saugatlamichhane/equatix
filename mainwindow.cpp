#include "mainwindow.h"
#include "BoardView.h"
#include "RackView.h"
#include "TileBag.h"
#include "EquationValidator.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QMessageBox>
#include <QStatusBar>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_board(new BoardView(11, this)),
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
    QAction *refill = new QAction("Refill Rack", this);
    toolbar->addAction(validate);
    toolbar->addAction(undo);
    toolbar->addAction(refill);

    connect(validate, &QAction::triggered, this, &MainWindow::onValidate);
    connect(undo, &QAction::triggered, this, &MainWindow::onUndo);
    connect(refill, &QAction::triggered, this, &MainWindow::onRefill);

    setCentralWidget(central);
    statusBar()->showMessage("Drag tiles from rack to board. Place valid equations. '/' must divide exactly.");

    // fill initial rack
    refillToSeven();
}

void MainWindow::refillToSeven() {
    while (m_rack->countTiles() < 7) {
        QChar ch = m_bag->draw();
        if (ch.isNull()) break;
        m_rack->addTile(ch);
    }
}

QVector<QVector<QChar>> MainWindow::boardSnapshot() const {
    int R = m_board->rowCount(), C = m_board->columnCount();
    QVector<QVector<QChar>> snap(R, QVector<QChar>(C));
    for (int r=0;r<R;++r) for (int c=0;c<C;++c) {
            QTableWidgetItem *it = m_board->item(r,c);
            if (it && !it->text().isEmpty()) snap[r][c] = it->text().at(0);
            else snap[r][c] = QChar(); // null char
        }
    return snap;
}

void MainWindow::onValidate() {
    auto snap = boardSnapshot();
    QString why;
    if (!EquationValidator::validate(snap, m_board->newTiles(), why)) {
        QMessageBox::warning(this, "Invalid Turn", why);
        return;
    }
    m_board->lockNewTiles();
    refillToSeven();
    statusBar()->showMessage("Turn accepted ✔", 2000);
}

void MainWindow::onUndo() {
    QList<QChar> returned;
    m_board->rollbackNewTiles(returned);
    for (QChar c : returned) m_rack->addTile(c);
    statusBar()->showMessage("Undid placements", 1500);
}

void MainWindow::onRefill() {
    refillToSeven();
    statusBar()->showMessage("Refilled rack", 1000);
}

