#pragma once
#include <QString>
#include <QDate>
#include <QDateTime>
#include <QVector>

struct GameRecord {
    int id = 0;
    QString playerName;
    int score = 0, kills = 0, level = 0, playTimeSec = 0;
    QDateTime dateTime;
};

class Database {
public:
    static Database& instance();
    bool init();
    bool saveRecord(const QString& name, int score, int kills, int level, int playTimeSec);
    QVector<GameRecord> getRecords(const QString& nameFilter = QString(),
                                   const QDate& from = QDate(),
                                   const QDate& to   = QDate());
private:
    Database() = default;
    bool initialized = false;
};
