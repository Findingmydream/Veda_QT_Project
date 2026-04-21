#include "recordstable.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QStringList>
#include <QChar>
#include <QColor>
#include <QAbstractItemView>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QStyledItemDelegate>

namespace RecordsTable {
namespace {

bool isAiOpponent(const QString& opponent)
{
    QString name = opponent.trimmed();
    return name.compare(QStringLiteral("AI"), Qt::CaseInsensitive) == 0
        || name.startsWith(QStringLiteral("AI("), Qt::CaseInsensitive);
}

class HoverRowDelegate : public QStyledItemDelegate
{
public:
    explicit HoverRowDelegate(QObject* parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void setHoverRow(int row)
    {
        hoverRow_ = row;
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        QStyleOptionViewItem opt(option);
        if (index.row() == hoverRow_ && !(opt.state & QStyle::State_Selected)) {
            painter->fillRect(opt.rect, QColor("#1a4a80"));
            opt.features &= ~QStyleOptionViewItem::Alternate;
            opt.backgroundBrush = Qt::NoBrush;
        }

        QStyledItemDelegate::paint(painter, opt, index);
    }

private:
    int hoverRow_ = -1;
};

class HoverRowEventFilter : public QObject
{
public:
    HoverRowEventFilter(QTableWidget* table, HoverRowDelegate* delegate)
        : QObject(table)
        , table_(table)
        , delegate_(delegate)
    {
    }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        if (watched == table_->viewport()) {
            if (event->type() == QEvent::MouseMove) {
                auto* mouse = static_cast<QMouseEvent*>(event);
                delegate_->setHoverRow(table_->indexAt(mouse->pos()).row());
                table_->viewport()->update();
            } else if (event->type() == QEvent::Leave) {
                delegate_->setHoverRow(-1);
                table_->viewport()->update();
            }
        }
        return QObject::eventFilter(watched, event);
    }

private:
    QTableWidget* table_ = nullptr;
    HoverRowDelegate* delegate_ = nullptr;
};

}  // namespace

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

void setupColumns(QTableWidget* table, bool includePlayerColumn, bool fillWidth)
{
    QStringList headers;
    headers << "ID";
    if (includePlayerColumn) headers << "플레이어";
    headers << "모드" << "결과"
            << "내 칭호" << "대결" << "상대 칭호"
            << "상대"
            << "내 돌" << "대결" << "상대 돌"
            << "수" << "게임시간" << "날짜";

    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->setColumnHidden(0, true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setVisible(false);
    table->setMouseTracking(true);
    table->viewport()->setMouseTracking(true);
    auto* hoverDelegate = new HoverRowDelegate(table);
    table->setItemDelegate(hoverDelegate);
    table->viewport()->installEventFilter(new HoverRowEventFilter(table, hoverDelegate));

    auto* hh = table->horizontalHeader();
    hh->setStretchLastSection(false);
    table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    for (int c = 0; c < table->columnCount(); ++c)
        hh->setSectionResizeMode(c, QHeaderView::ResizeToContents);

    if (fillWidth) {
        // "상대" (이름) 컬럼을 Stretch 로 전환 → 넓은 탭에선 남는 공간을 채움.
        // 새 컬럼 순서: ID / [플레이어] / 모드 / 결과 / 내칭호 / 대결 / 상대칭호 / 상대 / ...
        int oppNameCol = includePlayerColumn ? 7 : 6;
        hh->setSectionResizeMode(oppNameCol, QHeaderView::Stretch);
    }
    // fillWidth=false 면 전부 ResizeToContents → 내용 넘치면 자동 가로 스크롤.
}

void fill(QTableWidget* table, const QVector<GameRecord>& records, bool includePlayerColumn)
{
    // ResizeToContents 모드에서 setItem 마다 너비 재계산이 일어나 O(행×열) 이 됨.
    // 채우는 동안 Interactive 로 바꿔두고 마지막에 한 번만 내용 크기로 리사이즈.
    auto* hh = table->horizontalHeader();
    QVector<QHeaderView::ResizeMode> savedModes;
    savedModes.reserve(table->columnCount());
    for (int c = 0; c < table->columnCount(); ++c) {
        savedModes.append(hh->sectionResizeMode(c));
        hh->setSectionResizeMode(c, QHeaderView::Interactive);
    }
    table->setUpdatesEnabled(false);
    const bool wasSorting = table->isSortingEnabled();
    table->setSortingEnabled(false);

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
        bool isAI = isAiOpponent(r.opponent);
        QString mode  = isAI ? "싱글(AI)" : "멀티";
        QString opp   = isAI && r.opponent.compare(QStringLiteral("AI"), Qt::CaseInsensitive) == 0
            ? QStringLiteral("AI(미상)")
            : r.opponent;

        auto stoneStr = [](int s) {
            return (s == 1) ? QStringLiteral("흑")
                 : (s == 2) ? QStringLiteral("백")
                 : QStringLiteral("-");
        };
        QString myStone  = stoneStr(r.myStone);
        int oppStoneVal  = (r.myStone == 1) ? 2 : (r.myStone == 2) ? 1 : 0;
        QString oppStone = stoneStr(oppStoneVal);

        QString myTitle  = r.myTitle.isEmpty() ? QStringLiteral("-") : r.myTitle;
        QString oppTitle = r.opponentTitle.isEmpty() ? QStringLiteral("-") : r.opponentTitle;

        const QString VS = QStringLiteral("VS");

        int col = 0;
        setC(col++, QString::number(r.id));
        if (includePlayerColumn) setC(col++, r.playerName);
        setC(col++, mode);
        setC(col++, r.result, rc);
        setC(col++, myTitle);
        setC(col++, VS, QColor(255, 200, 80));
        setC(col++, oppTitle);
        setC(col++, opp);
        setC(col++, myStone);
        setC(col++, VS, QColor(255, 200, 80));
        setC(col++, oppStone);
        setC(col++, QString::number(r.moves));
        setC(col++, formatDuration(r.durationSeconds));
        setC(col++, r.playedAt.toString("yyyy-MM-dd"));
    }

    // 모드 복구: 채우기 다 끝난 뒤 한 번만 재측정됨.
    for (int c = 0; c < table->columnCount(); ++c)
        hh->setSectionResizeMode(c, savedModes[c]);
    table->setSortingEnabled(wasSorting);
    table->setUpdatesEnabled(true);
}

}  // namespace RecordsTable
