#ifndef RACKVIEW_H
#define RACKVIEW_H

#include <QWidget>
#include <QVector>
#include <QChar>

class TileLabel;

class RackView : public QWidget {
    Q_OBJECT
public:
    explicit RackView(QWidget *parent=nullptr);
    int countTiles() const;
    void addTile(QChar ch);
    void addTiles(const QVector<QChar>& chars);
signals:
    void rackChanged();
private:
    void tidy();
};

#endif // RACKVIEW_H

