#ifndef TILELABEL_H
#define TILELABEL_H

#include <QLabel>
#include <QChar>

class TileLabel : public QLabel {
    Q_OBJECT
public:
    explicit TileLabel(QChar ch, QWidget *parent=nullptr);
    QChar tileChar() const { return m_ch; }

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    QChar m_ch;
    void styleMe();
};

#endif // TILELABEL_H
