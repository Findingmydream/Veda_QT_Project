#include "database.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QCryptographicHash>
#include <QRandomGenerator>

Database& Database::instance() { static Database inst; return inst; }

bool Database::init()
{
    if (initialized) return true;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(path);
    db.setDatabaseName(path + "/omok.db");
    if (!db.open()) { qDebug() << db.lastError(); return false; }

    QSqlQuery q;
    if (!q.exec("CREATE TABLE IF NOT EXISTS players ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "nickname TEXT UNIQUE NOT NULL,"
           "comment TEXT DEFAULT '',"
           "password_hash TEXT DEFAULT '',"
           "password_salt TEXT DEFAULT '',"
           "wins INTEGER DEFAULT 0,"
           "losses INTEGER DEFAULT 0,"
           "draws INTEGER DEFAULT 0,"
           "created_at TEXT)")) {
        qDebug() << q.lastError();
        return false;
    }

    if (!ensurePlayerPasswordColumns()) return false;
    if (!ensurePlayerAvatarColumn())    return false;

    if (!q.exec("CREATE TABLE IF NOT EXISTS records ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "player_id INTEGER,"
           "player_name TEXT,"
           "opponent TEXT,"
           "result TEXT,"
           "moves INTEGER DEFAULT 0,"
           "duration_seconds INTEGER DEFAULT 0,"
           "my_stone INTEGER DEFAULT 0,"
           "played_at TEXT)")) {
        qDebug() << q.lastError();
        return false;
    }
    if (!ensureRecordColumns()) return false;

    if (!q.exec("CREATE TABLE IF NOT EXISTS friends ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "player_id INTEGER,"
           "friend_name TEXT,"
           "favorite INTEGER DEFAULT 0,"
           "added_at TEXT)")) {
        qDebug() << q.lastError();
        return false;
    }

    initialized = true;
    return true;
}

bool Database::ensurePlayerPasswordColumns()
{
    bool hasHash = false;
    bool hasSalt = false;

    QSqlQuery info("PRAGMA table_info(players)");
    while (info.next()) {
        QString name = info.value("name").toString();
        if (name == "password_hash") hasHash = true;
        if (name == "password_salt") hasSalt = true;
    }

    QSqlQuery q;
    if (!hasHash && !q.exec("ALTER TABLE players ADD COLUMN password_hash TEXT DEFAULT ''"))
        return false;
    if (!hasSalt && !q.exec("ALTER TABLE players ADD COLUMN password_salt TEXT DEFAULT ''"))
        return false;

    return true;
}

bool Database::ensurePlayerAvatarColumn()
{
    bool hasAvatar = false;
    QSqlQuery info("PRAGMA table_info(players)");
    while (info.next()) {
        if (info.value("name").toString() == "avatar_path") { hasAvatar = true; break; }
    }
    if (hasAvatar) return true;
    QSqlQuery q;
    return q.exec("ALTER TABLE players ADD COLUMN avatar_path TEXT DEFAULT ''");
}

bool Database::ensureRecordColumns()
{
    bool hasDur = false, hasStone = false;
    QSqlQuery info("PRAGMA table_info(records)");
    while (info.next()) {
        QString name = info.value("name").toString();
        if (name == "duration_seconds") hasDur   = true;
        if (name == "my_stone")         hasStone = true;
    }
    QSqlQuery q;
    if (!hasDur && !q.exec("ALTER TABLE records ADD COLUMN duration_seconds INTEGER DEFAULT 0"))
        return false;
    if (!hasStone && !q.exec("ALTER TABLE records ADD COLUMN my_stone INTEGER DEFAULT 0"))
        return false;
    return true;
}

QString Database::makePasswordSalt() const
{
    return QString("%1%2")
        .arg(QDateTime::currentMSecsSinceEpoch(), 0, 16)
        .arg(QRandomGenerator::global()->generate64(), 0, 16);
}

QString Database::passwordHash(const QString& password, const QString& salt) const
{
    QByteArray data = (salt + ":" + password).toUtf8();
    return QString::fromLatin1(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
}

// ── Player CRUD ───────────────────────────────────────────────────────────────
bool Database::createPlayer(const QString& nickname, const QString& comment)
{
    QSqlQuery q;
    q.prepare("INSERT INTO players (nickname, comment, created_at) VALUES (:n,:c,:d)");
    q.bindValue(":n", nickname);
    q.bindValue(":c", comment);
    q.bindValue(":d", QDateTime::currentDateTime().toString(Qt::ISODate));
    return q.exec();
}

bool Database::createPlayerAccount(const QString& nickname, const QString& password,
                                   const QString& comment)
{
    QString salt = makePasswordSalt();
    QSqlQuery q;
    q.prepare("INSERT INTO players (nickname, comment, password_hash, password_salt, created_at) "
              "VALUES (:n,:c,:h,:s,:d)");
    q.bindValue(":n", nickname);
    q.bindValue(":c", comment);
    q.bindValue(":h", passwordHash(password, salt));
    q.bindValue(":s", salt);
    q.bindValue(":d", QDateTime::currentDateTime().toString(Qt::ISODate));
    return q.exec();
}

bool Database::setPlayerPassword(int id, const QString& password)
{
    QString salt = makePasswordSalt();
    QSqlQuery q;
    q.prepare("UPDATE players SET password_hash=:h, password_salt=:s WHERE id=:id");
    q.bindValue(":h", passwordHash(password, salt));
    q.bindValue(":s", salt);
    q.bindValue(":id", id);
    return q.exec();
}

bool Database::authenticatePlayer(const QString& nickname, const QString& password, Player* player)
{
    QSqlQuery q;
    q.prepare("SELECT * FROM players WHERE nickname=:n");
    q.bindValue(":n", nickname);
    q.exec();
    if (!q.next()) return false;

    QString salt = q.value("password_salt").toString();
    QString expectedHash = q.value("password_hash").toString();
    if (salt.isEmpty() || expectedHash.isEmpty()) return false;
    if (passwordHash(password, salt) != expectedHash) return false;

    if (player) {
        player->id = q.value("id").toInt();
        player->nickname = q.value("nickname").toString();
        player->comment  = q.value("comment").toString();
        player->hasPassword = true;
        player->avatarPath  = q.value("avatar_path").toString();
        player->wins     = q.value("wins").toInt();
        player->losses   = q.value("losses").toInt();
        player->draws    = q.value("draws").toInt();
        player->createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
    }
    return true;
}

Player Database::readPlayer(int id)
{
    Player p;
    QSqlQuery q;
    q.prepare("SELECT * FROM players WHERE id=:id");
    q.bindValue(":id", id);
    q.exec();
    if (q.next()) {
        p.id = q.value("id").toInt();
        p.nickname = q.value("nickname").toString();
        p.comment  = q.value("comment").toString();
        p.hasPassword = !q.value("password_hash").toString().isEmpty();
        p.avatarPath  = q.value("avatar_path").toString();
        p.wins     = q.value("wins").toInt();
        p.losses   = q.value("losses").toInt();
        p.draws    = q.value("draws").toInt();
        p.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
    }
    return p;
}

Player Database::readPlayerByName(const QString& nickname)
{
    Player p;
    QSqlQuery q;
    q.prepare("SELECT * FROM players WHERE nickname=:n");
    q.bindValue(":n", nickname);
    q.exec();
    if (q.next()) {
        p.id = q.value("id").toInt();
        p.nickname = q.value("nickname").toString();
        p.comment  = q.value("comment").toString();
        p.hasPassword = !q.value("password_hash").toString().isEmpty();
        p.avatarPath  = q.value("avatar_path").toString();
        p.wins     = q.value("wins").toInt();
        p.losses   = q.value("losses").toInt();
        p.draws    = q.value("draws").toInt();
        p.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
    }
    return p;
}

QVector<Player> Database::readAllPlayers()
{
    QVector<Player> list;
    QSqlQuery q("SELECT * FROM players ORDER BY wins DESC");
    while (q.next()) {
        Player p;
        p.id = q.value("id").toInt();
        p.nickname = q.value("nickname").toString();
        p.comment  = q.value("comment").toString();
        p.hasPassword = !q.value("password_hash").toString().isEmpty();
        p.avatarPath  = q.value("avatar_path").toString();
        p.wins     = q.value("wins").toInt();
        p.losses   = q.value("losses").toInt();
        p.draws    = q.value("draws").toInt();
        p.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
        list.append(p);
    }
    return list;
}

bool Database::updatePlayer(int id, const QString& nickname, const QString& comment)
{
    QSqlQuery q;
    q.prepare("UPDATE players SET nickname=:n, comment=:c WHERE id=:id");
    q.bindValue(":n", nickname);
    q.bindValue(":c", comment);
    q.bindValue(":id", id);
    return q.exec();
}

bool Database::deletePlayer(int id)
{
    QSqlQuery q;
    q.prepare("DELETE FROM players WHERE id=:id");
    q.bindValue(":id", id);
    return q.exec();
}

bool Database::setPlayerAvatar(int id, const QString& avatarPath)
{
    QSqlQuery q;
    q.prepare("UPDATE players SET avatar_path=:a WHERE id=:id");
    q.bindValue(":a",  avatarPath);
    q.bindValue(":id", id);
    return q.exec();
}

double Database::averageMoves(int playerId)
{
    QSqlQuery q;
    q.prepare("SELECT AVG(moves) FROM records WHERE player_id=:pid");
    q.bindValue(":pid", playerId);
    if (!q.exec() || !q.next()) return 0.0;
    return q.value(0).toDouble();
}

bool Database::playerExists(const QString& nickname)
{
    QSqlQuery q;
    q.prepare("SELECT id FROM players WHERE nickname=:n");
    q.bindValue(":n", nickname);
    q.exec();
    return q.next();
}

// ── GameRecord CRUD ───────────────────────────────────────────────────────────
bool Database::createRecord(int playerId, const QString& opponent,
                             const QString& result, int moves,
                             int durationSeconds, int myStone)
{
    // 플레이어 승/패/무 카운트 업데이트
    QString col = (result == "승") ? "wins" : (result == "패") ? "losses" : "draws";
    QSqlQuery upd;
    upd.prepare(QString("UPDATE players SET %1=%1+1 WHERE id=:id").arg(col));
    upd.bindValue(":id", playerId);
    upd.exec();

    Player p = readPlayer(playerId);
    QSqlQuery q;
    q.prepare("INSERT INTO records "
              "(player_id,player_name,opponent,result,moves,duration_seconds,my_stone,played_at) "
              "VALUES (:pid,:pn,:op,:r,:m,:dur,:st,:dt)");
    q.bindValue(":pid", playerId);
    q.bindValue(":pn",  p.nickname);
    q.bindValue(":op",  opponent);
    q.bindValue(":r",   result);
    q.bindValue(":m",   moves);
    q.bindValue(":dur", durationSeconds);
    q.bindValue(":st",  myStone);
    q.bindValue(":dt",  QDateTime::currentDateTime().toString(Qt::ISODate));
    return q.exec();
}

QVector<GameRecord> Database::readRecords(int playerId, const QString& result)
{
    QVector<GameRecord> list;
    QString sql = "SELECT * FROM records WHERE 1=1";
    if (playerId > 0)     sql += " AND player_id=:pid";
    if (!result.isEmpty()) sql += " AND result=:r";
    sql += " ORDER BY played_at DESC";

    QSqlQuery q;
    q.prepare(sql);
    if (playerId > 0)     q.bindValue(":pid", playerId);
    if (!result.isEmpty()) q.bindValue(":r",   result);
    q.exec();

    while (q.next()) {
        GameRecord r;
        r.id              = q.value("id").toInt();
        r.playerId        = q.value("player_id").toInt();
        r.playerName      = q.value("player_name").toString();
        r.opponent        = q.value("opponent").toString();
        r.result          = q.value("result").toString();
        r.moves           = q.value("moves").toInt();
        r.durationSeconds = q.value("duration_seconds").toInt();
        r.myStone         = q.value("my_stone").toInt();
        r.playedAt        = QDateTime::fromString(q.value("played_at").toString(), Qt::ISODate);
        list.append(r);
    }
    return list;
}

bool Database::deleteRecord(int id)
{
    QSqlQuery find;
    find.prepare("SELECT player_id, result FROM records WHERE id=:id");
    find.bindValue(":id", id);
    if (!find.exec() || !find.next()) return false;

    int playerId = find.value("player_id").toInt();
    QString result = find.value("result").toString();
    QString col = (result == "승") ? "wins" : (result == "패") ? "losses" : "draws";

    QSqlQuery upd;
    upd.prepare(QString("UPDATE players SET %1=CASE WHEN %1>0 THEN %1-1 ELSE 0 END WHERE id=:id").arg(col));
    upd.bindValue(":id", playerId);
    if (!upd.exec()) return false;

    QSqlQuery q;
    q.prepare("DELETE FROM records WHERE id=:id");
    q.bindValue(":id", id);
    return q.exec();
}

void Database::deleteAllRecords(int playerId)
{
    QSqlQuery q;
    q.prepare("DELETE FROM records WHERE player_id=:pid");
    q.bindValue(":pid", playerId);
    q.exec();

    QSqlQuery reset;
    reset.prepare("UPDATE players SET wins=0, losses=0, draws=0 WHERE id=:pid");
    reset.bindValue(":pid", playerId);
    reset.exec();
}

// ── Friend CRUD ───────────────────────────────────────────────────────────────
bool Database::createFriend(int playerId, const QString& friendName)
{
    QSqlQuery q;
    q.prepare("INSERT INTO friends (player_id,friend_name,added_at) VALUES (:pid,:fn,:d)");
    q.bindValue(":pid", playerId);
    q.bindValue(":fn",  friendName);
    q.bindValue(":d",   QDateTime::currentDateTime().toString(Qt::ISODate));
    return q.exec();
}

QVector<Friend> Database::readFriends(int playerId, bool favoriteOnly)
{
    QVector<Friend> list;
    QString sql = "SELECT * FROM friends WHERE player_id=:pid";
    if (favoriteOnly) sql += " AND favorite=1";
    sql += " ORDER BY favorite DESC, friend_name ASC";

    QSqlQuery q;
    q.prepare(sql);
    q.bindValue(":pid", playerId);
    q.exec();

    while (q.next()) {
        Friend f;
        f.id         = q.value("id").toInt();
        f.playerId   = q.value("player_id").toInt();
        f.friendName = q.value("friend_name").toString();
        f.favorite   = q.value("favorite").toBool();
        f.addedAt    = QDateTime::fromString(q.value("added_at").toString(), Qt::ISODate);
        list.append(f);
    }
    return list;
}

bool Database::updateFriendFavorite(int id, bool favorite)
{
    QSqlQuery q;
    q.prepare("UPDATE friends SET favorite=:f WHERE id=:id");
    q.bindValue(":f",  favorite ? 1 : 0);
    q.bindValue(":id", id);
    return q.exec();
}

bool Database::deleteFriend(int id)
{
    QSqlQuery q;
    q.prepare("DELETE FROM friends WHERE id=:id");
    q.bindValue(":id", id);
    return q.exec();
}
