#include "SwapDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>

SwapDialog::SwapDialog(const QList<QChar>& availableTiles, QWidget* parent) : QDialog(parent) {
    setWindowTitle("Swap Tiles");
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(new QLabel("Select tiles to return to the bag:"));

    auto* tilesLayout = new QHBoxLayout();
    for (QChar tile : availableTiles) {
        QCheckBox* cb = new QCheckBox(QString(tile));
        cb->setStyleSheet("font-size: 16px; font-weight: bold;");
        m_checkboxes.append(cb);
        tilesLayout->addWidget(cb);
    }
    mainLayout->addLayout(tilesLayout);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SwapDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SwapDialog::reject);
    mainLayout->addWidget(buttonBox);
}

QList<QChar> SwapDialog::getSelectedTiles() const {
    QList<QChar> selected;
    for (auto* cb : m_checkboxes) {
        if (cb->isChecked()) {
            selected.append(cb->text().at(0));
        }
    }
    return selected;
}
