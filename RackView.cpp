
#include "RackView.h"
#include "TileLabel.h"
#include <QHBoxLayout>

RackView::RackView(QWidget *parent) : QWidget(parent) {
    auto *h = new QHBoxLayout(this);
    h->setSpacing(8);
    h->setContentsMargins(6,6,6,6);
    setLayout(h);
}

int RackView::countTiles() const {
    return findChildren<TileLabel*>().size();
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

void RackView::tidy() {
    // layout auto-adjusts; nothing needed here
}
