#ifndef SWAPDIALOG_H
#define SWAPDIALOG_H

#include <QDialog>
#include <QList>
#include <QChar>
#include <QCheckBox>

class SwapDialog : public QDialog {
    Q_OBJECT
public:
    explicit SwapDialog(const QList<QChar>& availableTiles, QWidget* parent = nullptr);
    QList<QChar> getSelectedTiles() const;

private:
    QList<QCheckBox*> m_checkboxes;
};

#endif // SWAPDIALOG_H
