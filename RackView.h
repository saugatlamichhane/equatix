#ifndef RACKVIEW_H
#define RACKVIEW_H

#include <QWidget>
#include <QVector>
#include <QChar>
#include <QList>

class TileLabel;

class RackView : public QWidget {
    Q_OBJECT
public:
    explicit RackView(QWidget *parent=nullptr);
    int countTiles() const;
    int countNonEqualsTiles() const;
    bool hasEqualsTile() const;

    QList<QChar> nonEqualsTiles() const;
    void removeTiles(const QList<QChar>& charsToRemove);

    void addTile(QChar ch);
    void addTiles(const QVector<QChar>& chars);

signals:
    void rackChanged();
};

#endif // RACKVIEW_H
