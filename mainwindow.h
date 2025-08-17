
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class BoardView;
class RackView;
class TileBag;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent=nullptr);

private slots:
    void onValidate();
    void onUndo();
    void onRefill();

private:
    BoardView *m_board;
    RackView *m_rack;
    TileBag *m_bag;

    void refillToSeven();
    QVector<QVector<QChar>> boardSnapshot() const;
};

#endif // MAINWINDOW_H
