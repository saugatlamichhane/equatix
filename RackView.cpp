#include "RackView.h"
#include "TileLabel.h"
#include <QHBoxLayout>
#include <QDebug>

RackView::RackView(QWidget *parent) : QWidget(parent) {
    auto *h = new QHBoxLayout(this);
    h->setSpacing(8);
    h->setContentsMargins(6,6,6,6);
    setLayout(h);
}

int RackView::countTiles() const {
    return findChildren<TileLabel*>().size();
}

int RackView::countNonEqualsTiles() const {
    int count = 0;
    for (auto* tile : findChildren<TileLabel*>()) {
        if (tile->tileChar() != '=') {
            count++;
        }
    }
    return count;
}

bool RackView::hasEqualsTile() const {
    for (auto* tile : findChildren<TileLabel*>()) {
        if (tile->tileChar() == '=') {
            return true;
        }
    }
    return false;
}

void RackView::addTile(QChar ch) {
    auto *t = new TileLabel(ch, this);
    layout()->addWidget(t);
    connect(t, &QObject::destroyed, this, &RackView::rackChanged);
    emit rackChanged();
}

void RackView::addTiles(const QVector<QChar>& chars) {
    for (QChar c : chars) addTile(c);
}

QList<QChar> RackView::nonEqualsTiles() const {
    QList<QChar> tiles;
    for (auto* label : findChildren<TileLabel*>()) {
        if (label->tileChar() != '=') {
            tiles.append(label->tileChar());
        }
    }
    return tiles;
}

void RackView::removeTiles(const QList<QChar>& charsToRemove) {
    QList<TileLabel*> tiles = findChildren<TileLabel*>();
    QList<QChar> toRemove = charsToRemove;

    for (auto* tile : tiles) {
        if (toRemove.contains(tile->tileChar())) {
            tile->close(); // This will trigger its deletion
            toRemove.removeOne(tile->tileChar());
        }
    }
}
