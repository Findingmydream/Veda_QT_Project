#include "recordstable.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QStringList>
#include <QChar>
#include <QColor>
#include <QAbstractItemView>

namespace RecordsTable {

QString formatDuration(int sec)
{
    if (sec <= 0) return "-";
    int h = sec / 3600;
    int m = (sec % 3600) / 60;
    int s = sec % 60;
    if (h > 0)
        return QString("%1:%2:%3").arg(h)
            .arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
    return QString("%1:%2")
        .arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
}

void setupColumns(QTableWidget* table, bool includePlayerColumn)
{
    QStringList headers;
    headers << "ID";
    if (includePlayerColumn) headers << "플레이어";
    headers << "모드" << "결과" << "내 돌" << "상대" << "수" << "게임시간" << "날짜";

    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->setColumnHidden(0, true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setVisible(false);

    auto* hh = table->horizontalHeader();
    hh->setStretchLastSection(false);
    table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 다른 컬럼은 내용 크기에 맞춤. "상대" 컬럼만 Stretch 로 남는 공간을 채움.
    // 컬럼 순서: ID / [플레이어] / 모드 / 결과 / 내 돌 / 상대 / 수 / 게임시간 / 날짜
    int oppCol = includePlayerColumn ? 5 : 4;
    for (int c = 0; c < table->columnCount(); ++c)
        hh->setSectionResizeMode(c, QHeaderView::ResizeToContents);
    hh->setSectionResizeMode(oppCol, QHeaderView::Stretch);
}

void fill(QTableWidget* table, const QVector<GameRecord>& records, bool includePlayerColumn)
{
    table->setRowCount(records.size());
    for (int i = 0; i < records.size(); ++i) {
        const auto& r = records[i];
        auto setC = [&](int col, const QString& t, QColor color = Qt::white) {
            auto* it = new QTableWidgetItem(t);
            it->setTextAlignment(Qt::AlignCenter);
            it->setForeground(color);
            table->setItem(i, col, it);
        };
        QColor rc = (r.result=="승") ? QColor(100,220,100)
                  : (r.result=="패") ? QColor(220,100,100)
                  : QColor(200,200,100);
        bool isAI = (r.opponent.compare("AI", Qt::CaseInsensitive) == 0);
        QString mode  = isAI ? "싱글(AI)" : "멀티";
        QString opp   = isAI ? "AI" : r.opponent;
        QString stone = (r.myStone == 1) ? "흑"
                      : (r.myStone == 2) ? "백" : "-";

        int col = 0;
        setC(col++, QString::number(r.id));
        if (includePlayerColumn) setC(col++, r.playerName);
        setC(col++, mode);
        setC(col++, r.result, rc);
        setC(col++, stone);
        setC(col++, opp);
        setC(col++, QString::number(r.moves));
        setC(col++, formatDuration(r.durationSeconds));
        setC(col++, r.playedAt.toString("yyyy-MM-dd"));
    }
}

}  // namespace RecordsTable
