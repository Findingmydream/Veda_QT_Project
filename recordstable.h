#pragma once
#include <QVector>
#include "database.h"

class QTableWidget;

namespace RecordsTable {

QString formatDuration(int seconds);

// 컬럼/헤더/스크롤 모드 셋업. ID는 0번 (숨김).
// includePlayerColumn=true 면 ID 다음에 "플레이어" 컬럼 추가.
void setupColumns(QTableWidget* table, bool includePlayerColumn);

// 행을 채움. setupColumns가 먼저 호출돼 있어야 함.
void fill(QTableWidget* table, const QVector<GameRecord>& records, bool includePlayerColumn);

}  // namespace RecordsTable
