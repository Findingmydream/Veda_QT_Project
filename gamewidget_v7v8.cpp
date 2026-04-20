#include "gamewidget.h"
#include "networkmanager.h"

#include <QKeyEvent>
#include <QPolygonF>
#include <QRandomGenerator>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QtMath>

static const QColor ENEMY_PALETTE[] = {
    QColor(255,  80,  80),   // lv1 – red
    QColor(255, 160,   0),   // lv2 – orange
    QColor(160,   0, 255),   // lv3 – purple
    QColor(  0, 200, 100),   // lv4 – green
    QColor(255,  50, 200),   // lv5+ – pink
};

// ─────────────────────────────────────────────────────────────────────────────
GameWidget::GameWidget(QWidget* parent)
    : QGraphicsView(parent)
    , scene    (new QGraphicsScene(this))
    , tickTimer (new QTimer(this))
    , spawnTimer(new QTimer(this))
    , shootTimer(new QTimer(this))
{
    setScene(scene);
    setFixedSize(W + 2, H + 2);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::Antialiasing);
    setFocusPolicy(Qt::StrongFocus);
    setFrameShape(QFrame::NoFrame);

    initScene();

    connect(tickTimer,  &QTimer::timeout, this, &GameWidget::tick);
    connect(spawnTimer, &QTimer::timeout, this, &GameWidget::spawnEnemy);
    connect(shootTimer, &QTimer::timeout, this, &GameWidget::autoShoot);
}

void GameWidget::setDifficulty(int difficulty)
{
    difficulty_ = qBound(1, difficulty, 3);
    if (spawnTimer->isActive())
        spawnTimer->setInterval(getSpawnInterval());
}

// ─────────────────────────────────────────────────────────────────────────────
void GameWidget::initScene()
{
    scene->setSceneRect(0, 0, W, H);
    scene->setBackgroundBrush(QColor(8, 8, 25));

    for (int i = 0; i < 120; ++i) {
        int x = QRandomGenerator::global()->bounded(W);
        int y = QRandomGenerator::global()->bounded(H);
        int a = QRandomGenerator::global()->bounded(60, 240);
        int s = QRandomGenerator::global()->bounded(1, 3);
        scene->addEllipse(x, y, s, s,
            QPen(Qt::transparent),
            QBrush(QColor(200, 220, 255, a)))->setZValue(0);
    }

    QFont f("Arial", 11, QFont::Bold);

    hudScore = scene->addText("SCORE: 0", f);
    hudScore->setDefaultTextColor(Qt::white);
    hudScore->setPos(6, 4);
    hudScore->setZValue(10);

    hudLevel = scene->addText("LV: 1", f);
    hudLevel->setDefaultTextColor(QColor(0, 220, 255));
    hudLevel->setPos(W / 2 - 24, 4);
    hudLevel->setZValue(10);

    hudLives = scene->addText("♥ ♥ ♥", f);
    hudLives->setDefaultTextColor(QColor(255, 80, 80));
    hudLives->setPos(W - 90, 4);
    hudLives->setZValue(10);

    // 멀티플레이 우측 HUD (기본 숨김)
    hudRemote = scene->addText("", f);
    hudRemote->setDefaultTextColor(QColor(0, 255, 120));
    hudRemote->setPos(W - 10, 4); // updateHUD에서 위치 재조정
    hudRemote->setZValue(10);
    hudRemote->setVisible(false);

    scene->addLine(0, 26, W, 26, QPen(QColor(40, 40, 80), 1))->setZValue(9);

    QFont big("Arial", 14, QFont::Bold);
    overlay = scene->addText("이름 입력 후\n[▶ 시작] 버튼을 누르세요", big);
    overlay->setDefaultTextColor(Qt::yellow);
    overlay->setPos(W / 2.0 - overlay->boundingRect().width() / 2.0,
                    H / 2.0 - overlay->boundingRect().height() / 2.0);
    overlay->setZValue(20);
}

// ─────────────────────────────────────────────────────────────────────────────
void GameWidget::createLocalPlayer()
{
    QPolygonF ship;
    ship << QPointF(20,  0)
         << QPointF( 0, 42)
         << QPointF( 8, 32)
         << QPointF(20, 38)
         << QPointF(32, 32)
         << QPointF(40, 42);

    player = scene->addPolygon(ship,
        QPen(QColor(0, 200, 255), 1.5),
        QBrush(QColor(0, 100, 200)));
    player->setPos(W / 2.0 - 20, H - 68);
    player->setZValue(5);
}

void GameWidget::setupRemotePlayer()
{
    QPolygonF ship;
    ship << QPointF(20,  0)
         << QPointF( 0, 42)
         << QPointF( 8, 32)
         << QPointF(20, 38)
         << QPointF(32, 32)
         << QPointF(40, 42);

    // Remote player is green
    remoteShip = scene->addPolygon(ship,
        QPen(QColor(0, 255, 120), 1.5),
        QBrush(QColor(0, 160, 80)));
    remoteShip->setPos(W / 2.0 + 60, H - 68);
    remoteShip->setZValue(5);

    QFont nameFont("Arial", 8, QFont::Bold);
    remoteNameLabel = scene->addText(remoteName_, nameFont);
    remoteNameLabel->setDefaultTextColor(QColor(0, 255, 120));
    remoteNameLabel->setZValue(11);

    remoteScoreLabel = scene->addText("", nameFont);
    remoteScoreLabel->setDefaultTextColor(QColor(0, 255, 120));
    remoteScoreLabel->setZValue(11);
}

// ─────────────────────────────────────────────────────────────────────────────
void GameWidget::startGame(const QString& pName)
{
    netMode      = false;
    net          = nullptr;
    playerName_  = pName.isEmpty() ? "Anonymous" : pName;

    clearGame();
    score = 0; lives = 3; level = 1; kills = 0;
    nextEnemyId = 0; netKills = 0;

    createLocalPlayer();
    hudRemote->setVisible(false);
    updateHUD();
    overlay->setVisible(false);

    tickTimer ->start(16);
    spawnTimer->start(getSpawnInterval());
    shootTimer->start(260);
    elapsed.start();

    running = true;
    paused_ = false;
    setFocus();
}

// ─────────────────────────────────────────────────────────────────────────────
void GameWidget::startNetworkGame(bool asHost,
                                   NetworkManager* netMgr,
                                   const QString&  localName,
                                   const QString&  remoteName)
{
    netMode      = true;
    isHostPlayer = asHost;
    net          = netMgr;
    playerName_  = localName.isEmpty()  ? "Player1" : localName;
    remoteName_  = remoteName.isEmpty() ? "Player2" : remoteName;

    clearGame();
    score = 0; lives = 3; level = 1; kills = 0;
    nextEnemyId = 0; netKills = 0;
    remoteScore_ = 0; remoteLives_ = 3;
    tickCount = 0;

    createLocalPlayer();
    setupRemotePlayer();
    hudRemote->setVisible(true);
    updateHUD();
    overlay->setVisible(false);

    tickTimer ->start(16);
    if (asHost) spawnTimer->start(getSpawnInterval()); // only host spawns
    shootTimer->start(260);
    elapsed.start();

    running = true;
    paused_ = false;
    setFocus();
}

// ─────────────────────────────────────────────────────────────────────────────
void GameWidget::clearGame()
{
    tickTimer ->stop();
    spawnTimer->stop();
    shootTimer->stop();

    qDeleteAll(playerBullets); playerBullets.clear();
    qDeleteAll(remoteBullets); remoteBullets.clear();

    for (auto& e : enemies) delete e.item;
    enemies.clear();

    qDeleteAll(enemyBullets); enemyBullets.clear();

    if (player)          { delete player;          player          = nullptr; }
    if (remoteShip)      { delete remoteShip;      remoteShip      = nullptr; }
    if (remoteNameLabel) { delete remoteNameLabel; remoteNameLabel = nullptr; }
    if (remoteScoreLabel){ delete remoteScoreLabel;remoteScoreLabel= nullptr; }
}

// ─────────────────────────────────────────────────────────────────────────────
void GameWidget::pauseGame()
{
    if (!running || paused_) return;
    paused_ = true;
    tickTimer->stop(); spawnTimer->stop(); shootTimer->stop();
    showOverlay("⏸  일시정지\n\n[재개] 버튼을 누르세요");
}

void GameWidget::resumeGame()
{
    if (!running || !paused_) return;
    paused_ = false;
    tickTimer->start(16);
    if (!netMode || isHostPlayer) spawnTimer->start(getSpawnInterval());
    shootTimer->start(260);
    overlay->setVisible(false);
    setFocus();
}

void GameWidget::stopGame(const QString& message)
{
    running = false;
    paused_ = false;
    clearGame();
    hudRemote->setVisible(false);
    showOverlay(message);
}

bool GameWidget::isRunning() const { return running && !paused_; }
bool GameWidget::isPaused()  const { return paused_; }

// ─────────────────────────────────────────────────────────────────────────────
void GameWidget::keyPressEvent(QKeyEvent* e)
{
    keys.insert(e->key());
    QGraphicsView::keyPressEvent(e);
}

void GameWidget::keyReleaseEvent(QKeyEvent* e)
{
    keys.remove(e->key());
    QGraphicsView::keyReleaseEvent(e);
}

// ─────────────────────────────────────────────────────────────────────────────
void GameWidget::tick()
{
    if (!running || paused_) return;
    ++tickCount;

    // ── Move local player ────────────────────────────────────────────────────
    if (player) {
        qreal px = player->x();
        if ((keys.contains(Qt::Key_Left)  || keys.contains(Qt::Key_A)) && px > 0)
            player->setX(qMax(0.0, px - 5.0));
        if ((keys.contains(Qt::Key_Right) || keys.contains(Qt::Key_D)) && px < W - 40)
            player->setX(qMin((qreal)(W - 40), px + 5.0));
    }

    // ── Send own position to remote (every 2 ticks ≈ 32 ms) ─────────────────
    if (netMode && net && player && tickCount % 2 == 0) {
        QJsonObject m; m["t"] = "pos"; m["x"] = (int)player->x();
        net->sendJson(m);
    }

    // ── Update remote ship label positions ───────────────────────────────────
    if (remoteShip && remoteNameLabel)
        remoteNameLabel->setPos(remoteShip->x() + 2, remoteShip->y() - 18);
    if (remoteShip && remoteScoreLabel)
        remoteScoreLabel->setPos(remoteShip->x() + 2, remoteShip->y() + 48);

    // ── Move player bullets ──────────────────────────────────────────────────
    for (int i = playerBullets.size() - 1; i >= 0; --i) {
        playerBullets[i]->moveBy(0, -10);
        if (playerBullets[i]->y() < -12) {
            delete playerBullets[i];
            playerBullets.removeAt(i);
        }
    }

    // ── Move remote bullets (시각 표시용) ───────────────────────────────────
    for (int i = remoteBullets.size() - 1; i >= 0; --i) {
        remoteBullets[i]->moveBy(0, -10);
        if (remoteBullets[i]->y() < -12) {
            delete remoteBullets[i];
            remoteBullets.removeAt(i);
        }
    }

    // ── Move enemies & let them shoot ────────────────────────────────────────
    int shootChance = 2 + level * 2 + (difficulty_ - 1) * 5;
    for (int i = enemies.size() - 1; i >= 0; --i) {
        auto* e = enemies[i].item;
        e->moveBy(0, getEnemySpeed());

        // 멀티에서는 호스트만 적 총알 생성 판정을 내려 두 화면을 맞춘다.
        if ((!netMode || isHostPlayer) &&
            QRandomGenerator::global()->bounded(1000) < shootChance) {
            qreal bx = e->x() + 15;
            qreal by = e->y() + 35;
            spawnEnemyBullet(bx, by);

            if (netMode && net) {
                QJsonObject m;
                m["t"]  = "enemyFire";
                m["id"] = enemies[i].id;
                m["x"]  = bx;
                m["y"]  = by;
                net->sendJson(m);
            }
        }

        if (e->y() > H + 10) { delete e; enemies.removeAt(i); }
    }

    // ── Move enemy bullets ───────────────────────────────────────────────────
    qreal ebSpeed = 3.5 + level * 0.4;
    for (int i = enemyBullets.size() - 1; i >= 0; --i) {
        enemyBullets[i]->moveBy(0, ebSpeed);
        if (enemyBullets[i]->y() > H + 10) {
            delete enemyBullets[i]; enemyBullets.removeAt(i);
        }
    }

    checkCollisions();
}

// ─────────────────────────────────────────────────────────────────────────────
void GameWidget::checkCollisions()
{
    if (!player) return;
    QRectF pr = player->mapToScene(player->boundingRect()).boundingRect();

    // ── Player bullets vs enemies ────────────────────────────────────────────
    for (int bi = playerBullets.size() - 1; bi >= 0; --bi) {
        QRectF br  = playerBullets[bi]->sceneBoundingRect();
        bool   hit = false;

        for (int ei = enemies.size() - 1; ei >= 0; --ei) {
            if (!br.intersects(enemies[ei].item->sceneBoundingRect())) continue;

            int killedId = enemies[ei].id;
            spawnExplosion(enemies[ei].item->x() + 15, enemies[ei].item->y() + 15);
            delete enemies[ei].item;   enemies.removeAt(ei);
            delete playerBullets[bi];  playerBullets.removeAt(bi);

            score += 10 * level;
            ++kills;

            if (netMode && net) {
                // Notify remote about the kill
                QJsonObject m;
                m["t"]     = "kill";
                m["id"]    = killedId;
                m["score"] = score;
                net->sendJson(m);

                // Host counts combined kills and owns level progression
                if (isHostPlayer) {
                    ++netKills;
                    if (netKills % 10 == 0) {
                        ++level;
                        spawnTimer->setInterval(getSpawnInterval());
                        QJsonObject lm; lm["t"] = "level"; lm["l"] = level;
                        net->sendJson(lm);
                    }
                }
            } else {
                // Single-player level
                if (kills % 10 == 0) {
                    ++level;
                    spawnTimer->setInterval(getSpawnInterval());
                }
            }

            updateHUD();
            hit = true;
            break;
        }
        if (hit) continue;
    }

    // ── Remote player bullets vs enemies (visual cleanup only) ───────────────
    for (int bi = remoteBullets.size() - 1; bi >= 0; --bi) {
        QRectF br = remoteBullets[bi]->sceneBoundingRect();
        for (int ei = enemies.size() - 1; ei >= 0; --ei) {
            if (!br.intersects(enemies[ei].item->sceneBoundingRect())) continue;
            delete remoteBullets[bi];
            remoteBullets.removeAt(bi);
            break;
        }
    }

    // ── Enemy bullets vs player ──────────────────────────────────────────────
    for (int i = enemyBullets.size() - 1; i >= 0; --i) {
        if (enemyBullets[i]->sceneBoundingRect().intersects(pr)) {
            delete enemyBullets[i]; enemyBullets.removeAt(i);
            loseLife();
            return;
        }
    }

    // ── Enemies vs player (direct) ───────────────────────────────────────────
    for (int i = enemies.size() - 1; i >= 0; --i) {
        if (!enemies[i].item->sceneBoundingRect().intersects(pr)) continue;

        int killedId = enemies[i].id;
        spawnExplosion(enemies[i].item->x() + 15, enemies[i].item->y() + 15);
        delete enemies[i].item; enemies.removeAt(i);

        if (netMode && net) {
            QJsonObject m; m["t"] = "kill"; m["id"] = killedId; m["score"] = score;
            net->sendJson(m);
        }
        loseLife();
        return;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void GameWidget::loseLife()
{
    --lives;
    updateHUD();

    if (netMode && net) {
        if (lives <= 0) {
            QJsonObject m; m["t"] = "dead"; m["score"] = score;
            net->sendJson(m);
        } else {
            QJsonObject m; m["t"] = "hit";
            net->sendJson(m);
        }
    }

    if (lives <= 0) {
        running = false;
        int playSec = (int)(elapsed.elapsed() / 1000);
        clearGame();

        if (netMode) {
            showOverlay(QString("💀 사망!\n\n내 최종 점수: %1\n\n상대방이 계속 싸우는 중...").arg(score));
        } else {
            showOverlay(QString("💀 게임 오버!\n\n최종 점수: %1\n처치: %2  레벨: %3\n\n[▶ 다시 시작] 버튼으로 재도전!")
                            .arg(score).arg(kills).arg(level));
        }
        emit gameOver(playerName_, score, kills, level, playSec);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
QGraphicsPolygonItem* GameWidget::makeEnemyItem(int x, int lv)
{
    QPolygonF shape;
    shape << QPointF(15,  0)
          << QPointF(30, 30)
          << QPointF(22, 20)
          << QPointF(15, 26)
          << QPointF( 8, 20)
          << QPointF( 0, 30);

    QColor c = ENEMY_PALETTE[qMin(lv - 1, 4)];
    auto*  e = scene->addPolygon(shape, QPen(c.lighter(150), 1.5), QBrush(c));
    e->setPos(x, -38);
    e->setZValue(5);
    return e;
}

void GameWidget::spawnEnemy()
{
    if (!running || paused_) return;

    int id = nextEnemyId++;
    int x  = QRandomGenerator::global()->bounded(10, W - 50);

    enemies.append({ id, makeEnemyItem(x, level) });

    // Host tells client to also spawn this enemy
    if (netMode && net && isHostPlayer) {
        QJsonObject m;
        m["t"]  = "spawn";
        m["id"] = id;
        m["x"]  = x;
        m["lv"] = level;
        net->sendJson(m);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void GameWidget::autoShoot()
{
    if (!running || paused_ || !player) return;
    auto* b = scene->addRect(-2, -10, 4, 14,
        QPen(Qt::transparent), QBrush(QColor(0, 240, 255)));
    b->setPos(player->x() + 18, player->y());
    b->setZValue(4);
    playerBullets.append(b);

    // 상대방에게 총알 위치 전송
    if (netMode && net) {
        QJsonObject m;
        m["t"] = "bullet";
        m["x"] = (int)(player->x() + 18);
        m["y"] = (int)player->y();
        net->sendJson(m);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Handle JSON arriving from the network peer
void GameWidget::handleNetMessage(const QJsonObject& obj)
{
    QString t = obj["t"].toString();

    // pause/resume 는 running 상태와 무관하게 처리
    if (t == "pause") {
        pauseGame();
        return;
    }
    if (t == "resume") {
        resumeGame();
        return;
    }

    if (!running && t != "dead") return;

    // ── Remote player position ───────────────────────────────────────────────
    if (t == "pos") {
        if (remoteShip) remoteShip->setX(obj["x"].toInt());
    }

    // ── Remote player fired a bullet → 초록 총알로 표시 ─────────────────────
    else if (t == "bullet") {
        auto* rb = scene->addRect(-2, -10, 4, 14,
            QPen(Qt::transparent), QBrush(QColor(0, 255, 120)));
        rb->setPos(obj["x"].toInt(), obj["y"].toInt());
        rb->setZValue(4);
        remoteBullets.append(rb);
    }

    // ── Host-authoritative enemy bullet ──────────────────────────────────────
    else if (t == "enemyFire" && !isHostPlayer) {
        spawnEnemyBullet(obj["x"].toDouble(), obj["y"].toDouble());
    }

    // ── Host spawns enemy → client creates it locally ────────────────────────
    else if (t == "spawn" && !isHostPlayer) {
        int id = obj["id"].toInt();
        int x  = obj["x"].toInt();
        int lv = obj["lv"].toInt(1);
        enemies.append({ id, makeEnemyItem(x, lv) });
    }

    // ── Remote player killed an enemy ────────────────────────────────────────
    else if (t == "kill") {
        int id = obj["id"].toInt();
        for (int i = enemies.size() - 1; i >= 0; --i) {
            if (enemies[i].id != id) continue;
            removeRemoteBulletNear(enemies[i].item->sceneBoundingRect().center());
            spawnExplosion(enemies[i].item->x() + 15, enemies[i].item->y() + 15);
            delete enemies[i].item;
            enemies.removeAt(i);
            break;
        }
        remoteScore_ = obj["score"].toInt();
        if (remoteScoreLabel)
            remoteScoreLabel->setPlainText(QString("P2: %1").arg(remoteScore_));

        // Host: count this kill toward level progression
        if (isHostPlayer) {
            ++netKills;
            if (netKills % 10 == 0) {
                ++level;
                spawnTimer->setInterval(getSpawnInterval());
                QJsonObject lm; lm["t"] = "level"; lm["l"] = level;
                net->sendJson(lm);
            }
            updateHUD();
        }
    }

    // ── Host broadcasts level up ─────────────────────────────────────────────
    else if (t == "level" && !isHostPlayer) {
        level = obj["l"].toInt();
        updateHUD();
    }

    // ── Remote lost a life ───────────────────────────────────────────────────
    else if (t == "hit") {
        remoteLives_ = qMax(0, remoteLives_ - 1);
        updateHUD();
    }

    // ── Host died ────────────────────────────────────────────────────────────
    else if (t == "dead") {
        remoteScore_ = obj["score"].toInt();
        remoteLives_ = 0;
        if (remoteShip)       remoteShip->setVisible(false);
        if (remoteNameLabel)  remoteNameLabel->setVisible(false);
        if (remoteScoreLabel) remoteScoreLabel->setVisible(false);
        updateHUD();
        // Local player continues until they also die
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void GameWidget::spawnExplosion(qreal x, qreal y)
{
    auto* exp = scene->addEllipse(-18, -18, 36, 36,
        QPen(Qt::transparent), QBrush(QColor(255, 200, 0, 210)));
    exp->setPos(x, y);
    exp->setZValue(8);
    QTimer::singleShot(130, this, [this, exp]() {
        scene->removeItem(exp);
        delete exp;
    });
}

void GameWidget::spawnEnemyBullet(qreal x, qreal y)
{
    auto* eb = scene->addEllipse(-4, -4, 8, 8,
        QPen(Qt::transparent), QBrush(QColor(255, 50, 50)));
    eb->setPos(x, y);
    eb->setZValue(4);
    enemyBullets.append(eb);
}

void GameWidget::removeRemoteBulletNear(const QPointF& scenePos)
{
    int bestIndex = -1;
    qreal bestDist2 = 45.0 * 45.0;

    for (int i = 0; i < remoteBullets.size(); ++i) {
        QPointF c = remoteBullets[i]->sceneBoundingRect().center();
        qreal dx = c.x() - scenePos.x();
        qreal dy = c.y() - scenePos.y();
        qreal dist2 = dx * dx + dy * dy;
        if (dist2 < bestDist2) {
            bestDist2 = dist2;
            bestIndex = i;
        }
    }

    if (bestIndex < 0) return;
    delete remoteBullets[bestIndex];
    remoteBullets.removeAt(bestIndex);
}

void GameWidget::updateHUD()
{
    if (netMode) {
        // 좌측: 내 정보 (파란색)
        hudScore->setPlainText(QString("🔵 %1\n%2  SCORE:%3")
            .arg(playerName_)
            .arg(QString("♥ ").repeated(qMax(0, lives)).trimmed())
            .arg(score));
        hudScore->setPos(4, 2);

        // 중앙: 레벨
        hudLevel->setPlainText(QString("LV:%1").arg(level));
        hudLevel->setPos(W / 2 - hudLevel->boundingRect().width() / 2, 4);

        // 우측: 상대방 정보 (초록색) - 우측 정렬
        QString remLives = QString("♥ ").repeated(qMax(0, remoteLives_)).trimmed();
        hudRemote->setPlainText(QString("🟢 %1\nSCORE:%2  %3")
            .arg(remoteName_)
            .arg(remoteScore_)
            .arg(remLives));
        hudRemote->setPos(W - hudRemote->boundingRect().width() - 4, 2);

        // 싱글플레이어용 hudLives는 멀티에서 숨김
        hudLives->setVisible(false);
    } else {
        // 싱글플레이어 기존 HUD
        hudScore->setPlainText(QString("SCORE: %1").arg(score));
        hudScore->setPos(6, 4);
        hudLevel->setPlainText(QString("LV: %1").arg(level));
        hudLevel->setPos(W / 2 - 24, 4);
        hudLives->setVisible(true);
        QString h;
        for (int i = 0; i < lives; ++i) h += "♥ ";
        hudLives->setPlainText(h.trimmed());
    }

    emit statsChanged(score, lives, level);
}

void GameWidget::showOverlay(const QString& msg)
{
    overlay->setPlainText(msg);
    overlay->setPos(W / 2.0 - overlay->boundingRect().width()  / 2.0,
                    H / 2.0 - overlay->boundingRect().height() / 2.0);
    overlay->setVisible(true);
}

int GameWidget::getSpawnInterval() const
{
    const int base = 1600 - (level - 1) * 180;
    const qreal multiplier = (difficulty_ == 1) ? 1.0 : (difficulty_ == 2 ? 0.82 : 0.62);
    return qMax(260, qRound(base * multiplier));
}

qreal GameWidget::getEnemySpeed() const
{
    const qreal bonus = (difficulty_ == 1) ? 0.0 : (difficulty_ == 2 ? 0.45 : 1.05);
    return 1.4 + (level - 1) * 0.55 + bonus;
}
