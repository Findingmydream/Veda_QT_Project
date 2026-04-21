#pragma once
#include <QString>
#include <QDateTime>
#include <QVector>

// ── 플레이어 프로필 ───────────────────────────────────────────────────────────
struct Player {
    int     id       = 0;
    QString nickname;
    QString comment; // (deprecated, kept for DB compat)
    QString avatarPath;
    bool    hasPassword = false;
    int     wins     = 0;
    int     losses   = 0;
    int     draws    = 0;
    QDateTime createdAt;

    int    totalGames() const { return wins + losses + draws; }
    double winRate()    const { int t = totalGames(); return t > 0 ? (wins * 100.0 / t) : 0.0; }
};

// ── 전적 기록 ─────────────────────────────────────────────────────────────────
struct GameRecord {
    int     id       = 0;
    int     playerId = 0;
    QString playerName;
    QString opponent;   // 상대 이름 or "AI"
    QString result;     // "승" "패" "무"
    int     moves    = 0;
    QDateTime playedAt;
};

// ── 친구 ──────────────────────────────────────────────────────────────────────
struct Friend {
    int     id         = 0;
    int     playerId   = 0;
    QString friendName;
    bool    favorite   = false;
    QDateTime addedAt;
};

class Database
{
public:
    static Database& instance();
    bool init();

    // ── Player CRUD ──────────────────────────────────────────────────────────
    bool           createPlayer(const QString& nickname, const QString& comment = "");
    bool           createPlayerAccount(const QString& nickname, const QString& password,
                                       const QString& comment = "");
    bool           setPlayerPassword(int id, const QString& password);
    bool           authenticatePlayer(const QString& nickname, const QString& password,
                                      Player* player = nullptr);
    Player         readPlayer(int id);
    Player         readPlayerByName(const QString& nickname);
    QVector<Player> readAllPlayers();
    bool           updatePlayer(int id, const QString& nickname, const QString& comment);
    bool           setPlayerAvatar(int id, const QString& avatarPath);
    bool           deletePlayer(int id);
    bool           playerExists(const QString& nickname);
    double         averageMoves(int playerId);

    // ── GameRecord CRUD ──────────────────────────────────────────────────────
    bool                  createRecord(int playerId, const QString& opponent,
                                       const QString& result, int moves);
    QVector<GameRecord>   readRecords(int playerId = 0,
                                      const QString& result = QString());
    bool                  deleteRecord(int id);
    void                  deleteAllRecords(int playerId);

    // ── Friend CRUD ──────────────────────────────────────────────────────────
    bool            createFriend(int playerId, const QString& friendName);
    QVector<Friend> readFriends(int playerId, bool favoriteOnly = false);
    bool            updateFriendFavorite(int id, bool favorite);
    bool            deleteFriend(int id);

private:
    Database() = default;
    bool ensurePlayerPasswordColumns();
    bool ensurePlayerAvatarColumn();
    QString makePasswordSalt() const;
    QString passwordHash(const QString& password, const QString& salt) const;
    bool initialized = false;
};
