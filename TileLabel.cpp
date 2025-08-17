#include "TileLabel.h"
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>

static const char* kMimeType = "application/x-equatix-tile";

TileLabel::TileLabel(QChar ch, QWidget *parent)
    : QLabel(parent), m_ch(ch)
{
    setText(QString(ch));
    setAlignment(Qt::AlignCenter);
    setFixedSize(44,44);
    setAttribute(Qt::WA_DeleteOnClose, true);
    styleMe();
}

void TileLabel::styleMe() {
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(2);
    setStyleSheet("font-weight: bold; font-size: 18px; background: #fff8dc;");
}

void TileLabel::mousePressEvent(QMouseEvent *event) {
    if (text().isEmpty()) { QLabel::mousePressEvent(event); return; }

    auto *mime = new QMimeData;
    QByteArray ba;
    ba.append(m_ch.toLatin1());
    mime->setData(kMimeType, ba);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mime);
    drag->setPixmap(grab());
    drag->setHotSpot(QPoint(width()/2, height()/2));

    if (drag->exec(Qt::MoveAction) == Qt::MoveAction) {
        // remove from rack when successfully dropped
        close(); // deletion via WA_DeleteOnClose
    }
}

