#pragma once
#include <QVector>
#include "database.h"

class QTableWidget;

namespace RecordsTable {

QString formatDuration(int seconds);

// 컬럼/헤더/스크롤 모드 셋업. ID는 0번 (숨김).
// includePlayerColumn=true 면 ID 다음에 "플레이어" 컬럼 추가.
// fillWidth=true 면 "상대" 컬럼이 남는 공간을 채움 (넓은 탭용).
// fillWidth=false 면 모든 컬럼이 내용 크기로 고정되어 내용 넘치면 가로 스크롤 (좁은 다이얼로그용).
void setupColumns(QTableWidget* table, bool includePlayerColumn, bool fillWidth = true);

// 행을 채움. setupColumns가 먼저 호출돼 있어야 함.
void fill(QTableWidget* table, const QVector<GameRecord>& records, bool includePlayerColumn);

}  // namespace RecordsTable
