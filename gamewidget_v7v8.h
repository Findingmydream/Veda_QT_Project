#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPolygonItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QTimer>
#include <QElapsedTimer>
#include <QSet>
#include <QVector>
#include <QJsonObject>

class NetworkManager;

// Each enemy carries an ID so network kill-sync can find it by ID
struct EnemyData {
    int                   id;
    QGraphicsPolygonItem* item;
};

class GameWidget : public QGraphicsView
{
    Q_OBJECT

public:
    explicit GameWidget(QWidget* parent = nullptr);

    void setDifficulty(int difficulty);

    // ── Single-player ─────────────────────────────────
    void startGame(const QString& playerName);

    // ── Multi-player ──────────────────────────────────
    void startNetworkGame(bool asHost,
                          NetworkManager* netMgr,
                          const QString&  localName,
                          const QString&  remoteName);

    void pauseGame();
    void resumeGame();
    void stopGame(const QString& message);

    bool isRunning() const;
    bool isPaused()  const;

signals:
    void gameOver(QString playerName, int score, int kills, int level, int playTimeSec);
    void statsChanged(int score, int lives, int level);

public slots:
    // Called by MainWindow with every incoming network JSON
    void handleNetMessage(const QJsonObject& obj);

protected:
    void keyPressEvent(QKeyEvent* e)   override;
    void keyReleaseEvent(QKeyEvent* e) override;

private slots:
    void tick();
    void spawnEnemy();
    void autoShoot();

private:
    // ── Setup / teardown ──────────────────────────────
    void initScene();
    void clearGame();
    void createLocalPlayer();
    void setupRemotePlayer();

    // ── Game logic ────────────────────────────────────
    void checkCollisions();
    void loseLife();
    void updateHUD();
    void showOverlay(const QString& msg);
    void spawnExplosion(qreal x, qreal y);
    void spawnEnemyBullet(qreal x, qreal y);
    void removeRemoteBulletNear(const QPointF& scenePos);

    // ── Enemy factory (shared between local & net mode)
    QGraphicsPolygonItem* makeEnemyItem(int x, int lv);

    int    getSpawnInterval() const;
    qreal  getEnemySpeed()    const;

    // ── Scene & timers ────────────────────────────────
    QGraphicsScene* scene;
    QTimer*         tickTimer;
    QTimer*         spawnTimer;
    QTimer*         shootTimer;
    QElapsedTimer   elapsed;

    // ── Local game objects ────────────────────────────
    QGraphicsPolygonItem*          player = nullptr;
    QVector<QGraphicsRectItem*>    playerBullets;
    QVector<QGraphicsRectItem*>    remoteBullets;   // 상대방 총알 (시각용)
    QVector<EnemyData>             enemies;
    QVector<QGraphicsEllipseItem*> enemyBullets;
    int nextEnemyId = 0;

    // ── HUD ───────────────────────────────────────────
    QGraphicsTextItem* hudScore;
    QGraphicsTextItem* hudLives;
    QGraphicsTextItem* hudLevel;
    QGraphicsTextItem* hudRemote = nullptr; // 멀티: 우측 상대방 정보
    QGraphicsTextItem* overlay;

    // ── State ─────────────────────────────────────────
    QString playerName_;
    int  score  = 0, lives  = 3, level  = 1, kills  = 0;
    int  difficulty_ = 1; // 1: easy, 2: normal, 3: hard
    bool running = false, paused_ = false;
    QSet<int> keys;

    // ── Network ───────────────────────────────────────
    NetworkManager* net          = nullptr;
    bool            netMode      = false;
    bool            isHostPlayer = false;
    int             tickCount    = 0;
    int             netKills     = 0; // host: combined kills (both players) for level

    // Remote player ship & labels
    QGraphicsPolygonItem* remoteShip       = nullptr;
    QGraphicsTextItem*    remoteNameLabel  = nullptr;
    QGraphicsTextItem*    remoteScoreLabel = nullptr;
    QString remoteName_;
    int     remoteScore_ = 0;
    int     remoteLives_ = 3;

    // ── Constants ─────────────────────────────────────
    static constexpr int   W = 400;
    static constexpr int   H = 530;
};
