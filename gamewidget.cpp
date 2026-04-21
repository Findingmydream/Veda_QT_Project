#include "gamewidget.h"
#include "networkmanager.h"

#include <QPainter>
#include <QMouseEvent>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QFont>
#include <QTimer>

GameWidget::GameWidget(QWidget* parent) : QWidget(parent)
{
    turnTimer_ = new QTimer(this);
    turnTimer_->setInterval(1000);
    connect(turnTimer_, &QTimer::timeout, this, &GameWidget::onTurnTimerTick);
    setFixedSize(sizeHint());
    resetBoard();
}

QSize GameWidget::sizeHint() const
{
    int sz = MARGIN * 2 + CELL * (BOARD - 1);
    return QSize(sz, sz);
}

// ── 시작 ─────────────────────────────────────────────────────────────────────
void GameWidget::startSingle(const QString& playerName, bool playerFirst)
{
    netMode  = false;
    net_     = nullptr;
    myName_  = playerName;
    opName_  = "AI";
    myStone  = playerFirst ? 1 : 2;
    resetBoard();
    playing  = true;
    gameOver_= false;
    paused_  = false;
    currentStone = 1;
    myTurn   = playerFirst;
    moveCount = 0;
    lastR = lastC = -1;
    forbiddenR = forbiddenC = -1;
    gameStartTime_ = QDateTime::currentDateTime();
    update();
    resetTurnTimer();

    if (!playerFirst) QTimer::singleShot(300, this, &GameWidget::doAiMove);
}

void GameWidget::startNetwork(int myStoneColor, NetworkManager* net,
                               const QString& myName, const QString& opName)
{
    netMode  = true;
    net_     = net;
    myName_  = myName;
    opName_  = opName;
    myStone  = (myStoneColor == 2) ? 2 : 1;
    resetBoard();
    playing  = true;
    gameOver_= false;
    paused_  = false;
    currentStone = 1;
    myTurn   = (myStone == 1);     // 흑이 먼저
    moveCount = 0;
    lastR = lastC = -1;
    forbiddenR = forbiddenC = -1;
    gameStartTime_ = QDateTime::currentDateTime();
    update();
    resetTurnTimer();
}

bool GameWidget::isPlaying() const { return playing; }

bool GameWidget::isPaused() const { return paused_; }

void GameWidget::setPaused(bool paused)
{
    if (!playing || gameOver_) return;
    if (paused_ == paused) return;

    paused_ = paused;
    update();

    if (paused_) {
        if (turnTimer_) turnTimer_->stop();
    } else if (playing && !gameOver_ && turnTimer_) {
        turnTimer_->start();
    }

    if (!paused_ && !netMode && !myTurn)
        QTimer::singleShot(300, this, &GameWidget::doAiMove);
}

void GameWidget::resetToIdle()
{
    resetBoard();
    playing = false;
    gameOver_ = false;
    paused_ = false;
    winner_.clear();
    netMode = false;
    net_ = nullptr;
    currentStone = 1;
    myTurn = true;
    moveCount = 0;
    lastR = lastC = -1;
    forbiddenR = forbiddenC = -1;
    stopTurnTimer();
    update();
}

// ── 보드 초기화 ───────────────────────────────────────────────────────────────
void GameWidget::resetBoard()
{
    for (int r = 0; r < BOARD; ++r)
        for (int c = 0; c < BOARD; ++c)
            board[r][c] = 0;
}

// ── 그리기 ───────────────────────────────────────────────────────────────────
void GameWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 배경
    p.fillRect(rect(), QColor(220, 179, 92));

    // 격자
    QPen gridPen(QColor(80, 50, 10), 1);
    p.setPen(gridPen);
    for (int i = 0; i < BOARD; ++i) {
        int x = MARGIN + i * CELL;
        int y = MARGIN + i * CELL;
        p.drawLine(x, MARGIN, x, MARGIN + (BOARD-1)*CELL);
        p.drawLine(MARGIN, y, MARGIN + (BOARD-1)*CELL, y);
    }

    // 화점 (천원, 삼삼 등)
    int dots[] = {3, 7, 11};
    p.setBrush(QColor(80, 50, 10));
    p.setPen(Qt::NoPen);
    for (int dr : dots)
        for (int dc : dots)
            p.drawEllipse(QPoint(MARGIN + dc*CELL, MARGIN + dr*CELL), 4, 4);

    // 돌
    for (int r = 0; r < BOARD; ++r) {
        for (int c = 0; c < BOARD; ++c) {
            if (!board[r][c]) continue;
            int cx = MARGIN + c * CELL;
            int cy = MARGIN + r * CELL;
            int radius = CELL / 2 - 2;

            bool isLast = (r == lastR && c == lastC);

            if (board[r][c] == 1) {
                // 흑돌
                QRadialGradient g(cx - radius/4, cy - radius/4, radius);
                g.setColorAt(0, QColor(80, 80, 80));
                g.setColorAt(1, QColor(0, 0, 0));
                p.setBrush(g);
                p.setPen(Qt::NoPen);
            } else {
                // 백돌
                QRadialGradient g(cx - radius/4, cy - radius/4, radius);
                g.setColorAt(0, Qt::white);
                g.setColorAt(1, QColor(180, 180, 180));
                p.setBrush(g);
                p.setPen(QPen(QColor(100,100,100), 1));
            }
            p.drawEllipse(QPoint(cx, cy), radius, radius);

            // 마지막 착수 표시
            if (isLast) {
                p.setPen(QPen(QColor(255, 50, 50), 2));
                p.setBrush(Qt::NoBrush);
                p.drawEllipse(QPoint(cx, cy), radius/2, radius/2);
            }
        }
    }

    if (forbiddenR >= 0 && forbiddenC >= 0) {
        int cx = MARGIN + forbiddenC * CELL;
        int cy = MARGIN + forbiddenR * CELL;
        int mark = CELL / 3;
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(QPen(QColor(230, 40, 40), 4));
        p.setBrush(QColor(230, 40, 40, 45));
        p.drawEllipse(QPoint(cx, cy), CELL / 2 - 3, CELL / 2 - 3);
        p.drawLine(cx - mark, cy - mark, cx + mark, cy + mark);
        p.drawLine(cx + mark, cy - mark, cx - mark, cy + mark);
    }

    // 게임 오버 오버레이
    if (paused_ && playing && !gameOver_) {
        p.fillRect(rect(), QColor(0, 0, 0, 120));
        p.setPen(Qt::white);
        QFont f("Arial", 22, QFont::Bold);
        p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter, "일시정지");
    }

    if (gameOver_) {
        p.fillRect(rect(), QColor(0, 0, 0, 120));
        p.setPen(Qt::white);
        QFont f("Arial", 22, QFont::Bold);
        p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter,
                   winner_.isEmpty() ? "무승부!" : winner_ + " 승리!");
    }

    // 차례 표시 (좌측 상단)
    if (playing && !gameOver_) {
        QString turnText = myTurn ? "내 차례" : opName_ + " 차례";
        QColor  turnColor = (currentStone == myStone) ? QColor(50,180,50) : QColor(200,80,80);
        p.setPen(turnColor);
        QFont f2("Arial", 10, QFont::Bold);
        p.setFont(f2);
        p.drawText(4, 14, turnText);
    }
}

// ── 클릭 ─────────────────────────────────────────────────────────────────────
void GameWidget::mousePressEvent(QMouseEvent* e)
{
    if (!playing || gameOver_ || paused_ || !myTurn) return;

    int c = qRound((e->position().x() - MARGIN) / (double)CELL);
    int r = qRound((e->position().y() - MARGIN) / (double)CELL);
    if (r < 0 || r >= BOARD || c < 0 || c >= BOARD) return;
    if (board[r][c]) return;

    // 내 착수
    if (!placeStone(r, c)) return;

    // 네트워크 전송
    if (netMode && net_) {
        QJsonObject m;
        m["t"] = "move";
        m["r"] = r;
        m["c"] = c;
        net_->sendJson(m);
    }
}

// ── 착수 공통 처리 ────────────────────────────────────────────────────────────
bool GameWidget::placeStone(int r, int c)
{
    if (isForbiddenDoubleThree(r, c, currentStone)) {
        forbiddenR = r;
        forbiddenC = c;
        emit ruleViolation("흑은 삼삼(33) 금수 자리에 둘 수 없습니다.");
        update();
        QTimer::singleShot(900, this, [this, r, c] {
            if (forbiddenR == r && forbiddenC == c) {
                forbiddenR = forbiddenC = -1;
                update();
            }
        });
        return false;
    }

    forbiddenR = forbiddenC = -1;
    board[r][c] = currentStone;
    lastR = r; lastC = c;
    ++moveCount;

    if (checkWin(r, c, currentStone)) {
        endGame(currentStone);
        return true;
    }

    // 무승부 체크
    bool full = true;
    for (int i = 0; i < BOARD && full; ++i)
        for (int j = 0; j < BOARD && full; ++j)
            if (!board[i][j]) full = false;
    if (full) { endGame(0); return true; }

    // 차례 전환
    currentStone = (currentStone == 1) ? 2 : 1;
    myTurn = !myTurn;
    resetTurnTimer();
    update();

    // AI 차례
    if (!netMode && !myTurn)
        QTimer::singleShot(300, this, &GameWidget::doAiMove);

    return true;
}



// ── 승리 판정 ─────────────────────────────────────────────────────────────────
bool GameWidget::checkWin(int r, int c, int stone) const
{
    int dx[] = {1,0,1,1};
    int dy[] = {0,1,1,-1};

    for (int d = 0; d < 4; ++d) {
        int cnt = 1;
        for (int s : {1, -1}) {
            int nr = r, nc = c;
            while (true) {
                nr += dy[d]*s; nc += dx[d]*s;
                if (nr<0||nr>=BOARD||nc<0||nc>=BOARD) break;
                if (board[nr][nc] != stone) break;
                ++cnt;
            }
        }
        if (cnt >= 5) return true;
    }
    return false;
}

bool GameWidget::isForbiddenDoubleThree(int r, int c, int stone)
{
    if (stone != 1 || board[r][c] != 0) return false;

    board[r][c] = stone;
    bool makesFive = checkWin(r, c, stone);
    int openThreeCount = makesFive ? 0 : countOpenThreesAt(r, c);
    board[r][c] = 0;

    // 33과 동시에 5목이 완성되는 경우는 승리로 허용한다.
    return !makesFive && openThreeCount >= 2;
}

int GameWidget::countOpenThreesAt(int r, int c) const
{
    const int dr[] = {0, 1, 1, 1};
    const int dc[] = {1, 0, 1, -1};
    int total = 0;

    for (int d = 0; d < 4; ++d) {
        QVector<int> line;
        int center = 4;
        line.reserve(9);

        for (int offset = -4; offset <= 4; ++offset) {
            int nr = r + dr[d] * offset;
            int nc = c + dc[d] * offset;
            if (nr < 0 || nr >= BOARD || nc < 0 || nc >= BOARD)
                line.append(2); // 보드 밖은 막힌 칸으로 취급
            else
                line.append(board[nr][nc]);
        }

        if (isOpenThreeLine(line, center)) ++total;
    }

    return total;
}

bool GameWidget::isOpenThreeLine(const QVector<int>& line, int center) const
{
    // 열린 3은 한 수를 더 두면 양쪽이 열린 4가 되는 3으로 본다.
    for (int i = 0; i < line.size(); ++i) {
        if (line[i] != 0) continue;

        QVector<int> candidate = line;
        candidate[i] = 1;

        for (int start = 0; start + 3 < candidate.size(); ++start) {
            int end = start + 3;
            if (center < start || center > end) continue;

            bool four = true;
            for (int k = start; k <= end; ++k) {
                if (candidate[k] != 1) {
                    four = false;
                    break;
                }
            }
            if (!four) continue;

            if (start > 0 && end + 1 < candidate.size()
                && candidate[start - 1] == 0 && candidate[end + 1] == 0) {
                return true;
            }
        }
    }

    return false;
}

// ── 게임 종료 ─────────────────────────────────────────────────────────────────
void GameWidget::endGame(int winner)
{
    stopTurnTimer();
    gameOver_ = true;
    playing   = false;

    if (winner == 0)         winner_ = "";
    else if (winner == myStone) winner_ = myName_;
    else                        winner_ = opName_;

    update();

    QString result;
    if (winner == 0)          result = "무";
    else if (winner == myStone) result = "승";
    else                        result = "패";

    int duration = gameStartTime_.isValid()
        ? gameStartTime_.secsTo(QDateTime::currentDateTime()) : 0;
    emit gameFinished(winner_, moveCount, duration, myStone);
}

// ── 네트워크 메시지 처리 ──────────────────────────────────────────────────────
void GameWidget::handleNetMessage(const QJsonObject& obj)
{
    if (obj["t"].toString() == "move") {
        int r = obj["r"].toInt();
        int c = obj["c"].toInt();
        placeStone(r, c);
    }
    else if (obj["t"].toString() == "timeout") {
        int winnerStone = obj["winnerStone"].toInt();
        endGame(winnerStone);
    }
}

void GameWidget::resetTurnTimer()
{
    secondsLeft_ = 30;
    emit turnTimeChanged(secondsLeft_, false);

    if (turnTimer_) {
        turnTimer_->stop();
        if (playing && !gameOver_ && !paused_)
            turnTimer_->start();
    }
}

void GameWidget::stopTurnTimer()
{
    if (turnTimer_) turnTimer_->stop();
    secondsLeft_ = 30;
    emit turnTimeChanged(secondsLeft_, false);
}

void GameWidget::onTurnTimerTick()
{
    if (!playing || gameOver_ || paused_) return;

    --secondsLeft_;
    if (secondsLeft_ < 0) secondsLeft_ = 0;
    emit turnTimeChanged(secondsLeft_, secondsLeft_ <= 10);

    if (secondsLeft_ > 0) return;

    int winnerStone = (currentStone == 1) ? 2 : 1;
    QString loser = (currentStone == myStone) ? myName_ : opName_;
    emit turnTimedOut(loser + " 시간 초과");

    if (netMode && net_) {
        QJsonObject msg;
        msg["t"] = "timeout";
        msg["winnerStone"] = winnerStone;
        net_->sendJson(msg);
    }

    endGame(winnerStone);
}

// ── AI ───────────────────────────────────────────────────────────────────────
void GameWidget::doAiMove()
{
    if (!playing || gameOver_ || paused_) return;

    int bestR = -1, bestC = -1, bestScore = -1;
    int aiStone = (myStone == 1) ? 2 : 1;

    for (int r = 0; r < BOARD; ++r) {
        for (int c = 0; c < BOARD; ++c) {
            if (board[r][c]) continue;
            if (isForbiddenDoubleThree(r, c, aiStone)) continue;
            int sc = aiScore(r, c, aiStone) * 2 + aiScore(r, c, myStone);
            if (sc > bestScore) { bestScore = sc; bestR = r; bestC = c; }
        }
    }

    if (bestR == -1) {
        // 빈 칸 랜덤 선택
        QVector<QPair<int,int>> empties;
        for (int r=0;r<BOARD;++r)
            for(int c=0;c<BOARD;++c)
                if(!board[r][c] && !isForbiddenDoubleThree(r, c, aiStone)) empties.append({r,c});
        if (empties.isEmpty()) return;
        auto idx = QRandomGenerator::global()->bounded(empties.size());
        bestR = empties[idx].first;
        bestC = empties[idx].second;
    }

    placeStone(bestR, bestC);
}

int GameWidget::aiScore(int r, int c, int stone) const
{
    int dx[] = {1,0,1,1};
    int dy[] = {0,1,1,-1};
    int total = 0;

    const_cast<GameWidget*>(this)->board[r][c] = stone; // 임시 배치
    for (int d = 0; d < 4; ++d) {
        int cnt = 1;
        for (int s : {1, -1}) {
            int nr = r, nc = c;
            while (true) {
                nr += dy[d]*s; nc += dx[d]*s;
                if (nr<0||nr>=BOARD||nc<0||nc>=BOARD) break;
                if (board[nr][nc] != stone) break;
                ++cnt;
            }
        }
        if (cnt >= 5) total += 100000;
        else if (cnt == 4) total += 1000;
        else if (cnt == 3) total += 100;
        else if (cnt == 2) total += 10;
    }
    const_cast<GameWidget*>(this)->board[r][c] = 0; // 임시 배치 제거
    return total;
}
