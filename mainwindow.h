#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QChar>

class BoardView;
class RackView;
class TileBag;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onValidate();
    void onUndo();
    void onSwap();

private:
    // UI / game widgets
    BoardView *m_board;
    RackView *m_racks[2]; // two racks, index 0 = Player 1, index 1 = Player 2
    TileBag *m_bag;

    // game state
    int m_currentPlayer; // 0 or 1

    // helpers
    void refillRack(int player);                 // refill specific player's rack
    QVector<QVector<QChar>> boardSnapshot() const;
    void endTurn();                              // toggle players, enable appropriate rack, update status
    void enableRacksForCurrentPlayer();          // enable/disable racks according to current player
};

#endif // MAINWINDOW_H
