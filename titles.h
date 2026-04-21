#pragma once
#include <QString>

inline QString playerTitle(int totalGames, double winRatePercent)
{
    if (totalGames <= 10)            return QStringLiteral("🐣 오린이");
    if (winRatePercent <  35.0)      return QStringLiteral("🪨 돌을 쥔 자");
    if (winRatePercent <  50.0)      return QStringLiteral("⚔️ 중수");
    if (winRatePercent <  65.0)      return QStringLiteral("🧠 고수");
    if (winRatePercent <  80.0)      return QStringLiteral("🔥 오친자");
    if (winRatePercent <  90.0)      return QStringLiteral("👑 오목킹");
    return QStringLiteral("🦴 오스트랄로피테쿠스");
}

inline int titleRank(int totalGames, double winRatePercent)
{
    if (totalGames <= 10)            return 0;
    if (winRatePercent <  35.0)      return 1;
    if (winRatePercent <  50.0)      return 2;
    if (winRatePercent <  65.0)      return 3;
    if (winRatePercent <  80.0)      return 4;
    if (winRatePercent <  90.0)      return 5;
    return 6;
}
