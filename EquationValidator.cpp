#include "EquationValidator.h"
#include <QStack>
#include <cctype>

// helpers to find contiguous runs
static int leftmost(const QVector<QVector<QChar>>& b, int r, int c) {
    while (c-1 >= 0 && !b[r][c-1].isNull()) --c;
    return c;
}
static int topmost(const QVector<QVector<QChar>>& b, int r, int c) {
    while (r-1 >= 0 && !b[r-1][c].isNull()) --r;
    return r;
}

QString EquationValidator::readRunH(const QVector<QVector<QChar>>& b, int r, int c) {
    int start = leftmost(b, r, c);
    QString s;
    int cols = b[r].size();
    for (int j = start; j < cols && !b[r][j].isNull(); ++j) s.append(b[r][j]);
    return s;
}
QString EquationValidator::readRunV(const QVector<QVector<QChar>>& b, int r, int c) {
    int start = topmost(b, r, c);
    QString s;
    int rows = b.size();
    for (int i = start; i < rows && !b[i][c].isNull(); ++i) s.append(b[i][c]);
    return s;
}

// Very small expression evaluator: + - * / with precedence. Division must be exact integer.
static int precedence(QChar op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

std::optional<long long> EquationValidator::evalExpr(const QString& s) {
    // Tokenize: integers and operators and parentheses
    QVector<QString> tokens;
    int i=0, n=s.size();
    while (i < n) {
        if (s[i].isSpace()) { ++i; continue; }
        if (s[i].isDigit()) {
            int j=i;
            while (j<n && s[j].isDigit()) ++j;
            tokens.append(s.mid(i, j-i));
            i=j;
        } else {
            QChar ch = s[i++];
            if (QString("+-*/()").contains(ch)) {
                tokens.append(QString(ch));
            } else {
                return std::nullopt;
            }
        }
    }

    QStack<long long> vals;
    QStack<QChar> ops;
    auto apply = [&](QChar op)->bool {
        if (vals.size() < 2) return false;
        long long b = vals.pop();
        long long a = vals.pop();
        long long res = 0;
        if (op == '+') res = a + b;
        else if (op == '-') res = a - b;
        else if (op == '*') res = a * b;
        else if (op == '/') {
            if (b == 0) return false;
            if (a % b != 0) return false; // require exact division
            res = a / b;
        } else return false;
        vals.push(res);
        return true;
    };

    for (int t=0;t<tokens.size();++t) {
        const QString &tok = tokens[t];
        if (tok[0].isDigit()) {
            vals.push(tok.toLongLong());
        } else if (tok == "(") {
            ops.push('(');
        } else if (tok == ")") {
            while (!ops.isEmpty() && ops.top() != '(') {
                if (!apply(ops.pop())) return std::nullopt;
            }
            if (ops.isEmpty() || ops.pop() != '(') return std::nullopt;
        } else { // operator
            QChar op = tok[0];
            while (!ops.isEmpty() && ops.top() != '(' && precedence(ops.top()) >= precedence(op)) {
                if (!apply(ops.pop())) return std::nullopt;
            }
            ops.push(op);
        }
    }
    while (!ops.isEmpty()) {
        if (ops.top() == '(') return std::nullopt;
        if (!apply(ops.pop())) return std::nullopt;
    }
    if (vals.size() != 1) return std::nullopt;
    return vals.top();
}

bool EquationValidator::isTrueEquation(const QString& run, QString &why) {
    int eqCount = run.count('=');
    if (eqCount != 1) { why = "must contain exactly one '='"; return false; }
    int idx = run.indexOf('=');
    if (idx <= 0 || idx >= run.size()-1) { why = "both sides required"; return false; }
    QString L = run.left(idx);
    QString R = run.mid(idx+1);
    auto lv = evalExpr(L);
    if (!lv) { why = "LHS invalid"; return false; }
    auto rv = evalExpr(R);
    if (!rv) { why = "RHS invalid"; return false; }
    if (*lv != *rv) { why = QString("%1 != %2").arg(QString::number(*lv)).arg(QString::number(*rv)); return false; }
    return true;
}

bool EquationValidator::validate(const QVector<QVector<QChar>>& board,
                                 const QSet<QPair<int,int>>& newTiles,
                                 QString &errorMessage)
{
    if (newTiles.isEmpty()) { errorMessage = "Place at least one tile."; return false; }

    QSet<QString> checked;
    for (auto rc : newTiles) {
        int r = rc.first, c = rc.second;
        // horizontal run
        QString runH = readRunH(board, r, c);
        if (runH.size() >= 2 && runH.contains('=')) {
            QString key = QString("H%1:%2").arg(r).arg(runH);
            if (!checked.contains(key)) {
                checked.insert(key);
                QString why;
                if (!isTrueEquation(runH, why)) { errorMessage = QString("Row %1: '%2' -> %3").arg(r+1).arg(runH, why); return false; }
            }
        }
        // vertical run
        QString runV = readRunV(board, r, c);
        if (runV.size() >= 2 && runV.contains('=')) {
            QString key = QString("V%1:%2").arg(c).arg(runV);
            if (!checked.contains(key)) {
                checked.insert(key);
                QString why;
                if (!isTrueEquation(runV, why)) { errorMessage = QString("Col %1: '%2' -> %3").arg(c+1).arg(runV, why); return false; }
            }
        }
    }
    return true;
}
