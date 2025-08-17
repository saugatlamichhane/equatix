#ifndef EQUATIONVALIDATOR_H
#define EQUATIONVALIDATOR_H

#include <QVector>
#include <QChar>
#include <QString>
#include "BoardView.h"

class EquationValidator {
public:
    // Validate all runs affected by new placements on board
    // board: snapshot of chars (null QChar means empty)
    // newPlacements: list of placements (row,col,ch)
    // returns true if all affected runs with length>=2 that contain '=' are valid equations.
    static bool validate(const QVector<QVector<QChar>>& board,
                         const QSet<QPair<int,int>>& newTiles,
                         QString &errorMessage);
private:
    static QString readRunH(const QVector<QVector<QChar>>& b, int r, int c);
    static QString readRunV(const QVector<QVector<QChar>>& b, int r, int c);
    static bool isTrueEquation(const QString& run, QString &why);
    static std::optional<long long> evalExpr(const QString& s);
};

#endif // EQUATIONVALIDATOR_H
