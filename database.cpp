#include "database.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

Database& Database::instance() { static Database inst; return inst; }

bool Database::init() {
    if (initialized) return true;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(path);
    db.setDatabaseName(path + "/shooting_records.db");
    if (!db.open()) { qDebug() << db.lastError().text(); return false; }
    QSqlQuery q;
    q.exec("CREATE TABLE IF NOT EXISTS records ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "player_name TEXT, score INTEGER, kills INTEGER,"
           "level INTEGER, play_time INTEGER, date_time TEXT)");
    initialized = true;
    return true;
}

bool Database::saveRecord(const QString& name, int score, int kills, int level, int playTimeSec) {
    QSqlQuery q;
    q.prepare("INSERT INTO records (player_name,score,kills,level,play_time,date_time) "
              "VALUES (:n,:s,:k,:l,:t,:d)");
    q.bindValue(":n", name); q.bindValue(":s", score);
    q.bindValue(":k", kills); q.bindValue(":l", level);
    q.bindValue(":t", playTimeSec);
    q.bindValue(":d", QDateTime::currentDateTime().toString(Qt::ISODate));
    return q.exec();
}

QVector<GameRecord> Database::getRecords(const QString& nameFilter, const QDate& from, const QDate& to) {
    QVector<GameRecord> results;
    QString sql = "SELECT id,player_name,score,kills,level,play_time,date_time FROM records WHERE 1=1";
    if (!nameFilter.isEmpty()) sql += " AND player_name LIKE :name";
    if (from.isValid())        sql += " AND date_time >= :from";
    if (to.isValid())          sql += " AND date_time <  :to";
    sql += " ORDER BY score DESC";
    QSqlQuery q; q.prepare(sql);
    if (!nameFilter.isEmpty()) q.bindValue(":name", "%" + nameFilter + "%");
    if (from.isValid())        q.bindValue(":from", from.toString(Qt::ISODate));
    if (to.isValid())          q.bindValue(":to",   to.addDays(1).toString(Qt::ISODate));
    q.exec();
    while (q.next()) {
        GameRecord r;
        r.id = q.value(0).toInt(); r.playerName = q.value(1).toString();
        r.score = q.value(2).toInt(); r.kills = q.value(3).toInt();
        r.level = q.value(4).toInt(); r.playTimeSec = q.value(5).toInt();
        r.dateTime = QDateTime::fromString(q.value(6).toString(), Qt::ISODate);
        results.append(r);
    }
    return results;
}
