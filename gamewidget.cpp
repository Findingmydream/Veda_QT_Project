#include "gamewidget.h"
#include "networkmanager.h"
#include "geminiclient.h"

#include <QPainter>
#include <QMouseEvent>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QFont>
#include <QTimer>
#include <QDebug>
#include <QRegularExpression>

GameWidget::GameWidget(QWidget* parent) : QWidget(parent)
{
    turnTimer_ = new QTimer(this);
    turnTimer_->setInterval(1000);
    connect(turnTimer_, &QTimer::timeout, this, &GameWidget::onTurnTimerTick);

    geminiClient_ = new GeminiClient(this);
    connect(geminiClient_, &GeminiClient::textReady, this, &GameWidget::onGeminiTextReady);
    connect(geminiClient_, &GeminiClient::failed,    this, &GameWidget::onGeminiFailed);

    setFixedSize(sizeHint());
    resetBoard();
}

QSize GameWidget::sizeHint() const
{
    int sz = MARGIN * 2 + CELL * (BOARD - 1);
    return QSize(sz, sz);
}

// ── 시작 ─────────────────────────────────────────────────────────────────────
void GameWidget::startSingle(const QString& playerName, bool playerFirst,
                              AIDifficulty difficulty)
{
    netMode  = false;
    net_     = nullptr;
    myName_  = playerName;
    opName_  = "AI";
    myStone  = playerFirst ? 1 : 2;
    aiDifficulty_ = difficulty;
    resetBoard();
    playing  = true;
    gameOver_= false;
    paused_  = false;
    currentStone = 1;
    myTurn   = playerFirst;
    moveCount = 0;
    lastR = lastC = -1;
    forbiddenR = forbiddenC = -1;
    forbiddenPreview_.clear();
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
    forbiddenPreview_.clear();
    gameStartTime_ = QDateTime::currentDateTime();
    refreshForbiddenPreview();
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
    forbiddenPreview_.clear();
    stopTurnTimer();
    update();
}

// ── 보드 초기화 ───────────────────────────────────────────────────────────────
void GameWidget::resetBoard()
{
    for (int r = 0; r < BOARD; ++r)
        for (int c = 0; c < BOARD; ++c)
            board[r][c] = 0;
    livesBlack_ = MAX_LIVES;
    livesWhite_ = MAX_LIVES;
    emit livesChanged(livesBlack_, livesWhite_);
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

    // 멀티에서 흑 차례 동안 모든 33 금수 자리를 옅게 미리 표시.
    if (!forbiddenPreview_.isEmpty() && playing && !gameOver_) {
        int mark = CELL / 4;
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(QPen(QColor(230, 40, 40, 170), 2));
        p.setBrush(QColor(230, 40, 40, 30));
        for (const QPoint& pt : forbiddenPreview_) {
            if (pt.x() == forbiddenC && pt.y() == forbiddenR) continue; // 클릭 시 강조된 자리는 아래에서 따로 그림
            int cx = MARGIN + pt.x() * CELL;
            int cy = MARGIN + pt.y() * CELL;
            p.drawEllipse(QPoint(cx, cy), CELL / 2 - 5, CELL / 2 - 5);
            p.drawLine(cx - mark, cy - mark, cx + mark, cy + mark);
            p.drawLine(cx + mark, cy - mark, cx - mark, cy + mark);
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
    if (r < 0 || r >= BOARD || c < 0 || c >= BOARD) return false;
    if (board[r][c] != 0) return false;

    qDebug() << "[placeStone] click at (" << r << "," << c << ") currentStone=" << currentStone
             << " myTurn=" << myTurn << " netMode=" << netMode;
    if (isForbiddenDoubleThree(r, c, currentStone)) {
        qDebug() << "[placeStone] -> FORBIDDEN detected";
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
    refreshForbiddenPreview();
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
    const char* dname[] = {"H ", "V ", "D\\", "D/"};
    int total = 0;

    qDebug().noquote() << "[33-check] at (" << r << "," << c << ") board[r][c]=" << board[r][c];
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

        bool ok = isOpenThreeLine(line, center);
        if (ok) ++total;

        QString s;
        for (int v : line) s += QString::number(v);
        qDebug().noquote() << "  dir" << dname[d] << "line=" << s << "openThree=" << ok;
    }

    qDebug().noquote() << "[33-check] total openThrees =" << total;
    return total;
}

void GameWidget::refreshForbiddenPreview()
{
    forbiddenPreview_.clear();
    // 유저 요구사항: 멀티 플레이 중 흑 차례일 때만 33 자리 프리뷰 표시.
    if (!netMode || !playing || gameOver_ || currentStone != 1) return;

    for (int r = 0; r < BOARD; ++r) {
        for (int c = 0; c < BOARD; ++c) {
            if (board[r][c] != 0) continue;
            if (isForbiddenDoubleThree(r, c, 1))
                forbiddenPreview_.append(QPoint(c, r));
        }
    }
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
        if (!netMode || !playing || gameOver_ || paused_ || myTurn) return;
        placeStone(r, c);
    }
    else if (obj["t"].toString() == "timeout") {
        int winnerStone = obj["winnerStone"].toInt();
        if (!playing || gameOver_ || (winnerStone != 1 && winnerStone != 2)) return;
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

    // 30초 만료: 현재 차례 플레이어가 목숨 하나 잃음.
    int& lives = (currentStone == 1) ? livesBlack_ : livesWhite_;
    if (lives > 0) --lives;
    emit livesChanged(livesBlack_, livesWhite_);

    if (lives > 0) {
        // 아직 목숨 남음 → 타이머만 리셋하고 게임 계속.
        QString loser = (currentStone == myStone) ? myName_ : opName_;
        emit turnTimedOut(QString("%1 시간 초과 (목숨 %2개 남음)").arg(loser).arg(lives));
        resetTurnTimer();
        return;
    }

    // 목숨 0 → 게임 종료.
    int winnerStone = (currentStone == 1) ? 2 : 1;
    QString loser = (currentStone == myStone) ? myName_ : opName_;
    emit turnTimedOut(loser + " 시간 초과 (목숨 소진)");

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

    // 첫 수(AI가 흑 선공): 중앙 ±2 범위에서 랜덤 착수.
    // 빈 보드에선 평가 함수가 전부 0점이라, 두면 항상 (0,0) 에서 시작하는 문제 방지.
    if (moveCount == 0) {
        int center = BOARD / 2;
        int rr = center + QRandomGenerator::global()->bounded(-2, 3);
        int cc = center + QRandomGenerator::global()->bounded(-2, 3);
        placeStone(rr, cc);
        return;
    }

    if      (aiDifficulty_ == AIDifficulty::Hard)   doAiMoveHard();
    else if (aiDifficulty_ == AIDifficulty::Medium) doAiMoveMedium();
    else                                             doAiMoveEasy();
}

// ── 어려움: Gemini API 호출 ──────────────────────────────────────────────────
void GameWidget::doAiMoveHard()
{
    if (aiThinking_) return;  // 중복 요청 방지

    int aiStone = (myStone == 1) ? 2 : 1;
    QString prompt = buildGeminiPrompt(aiStone);

    aiThinking_ = true;
    emit ruleViolation("🤖 AI 가 수를 고민 중...");
    geminiClient_->requestText(prompt);
}

QString GameWidget::buildGeminiPrompt(int aiStone) const
{
    QString sideName = (aiStone == 1) ? "BLACK (B)" : "WHITE (W)";
    QString myCh     = (aiStone == 1) ? "B" : "W";
    QString oppCh    = (aiStone == 1) ? "W" : "B";

    QString s;
    s += "You are playing Gomoku (Five in a Row) on a 15x15 board.\n";
    s += "You play as " + sideName + ". Your stones: " + myCh + ", opponent: " + oppCh + ", empty: .\n";
    s += "Win condition: exactly 5 stones in a row (horizontal, vertical, or diagonal).\n";
    if (aiStone == 1) {
        s += "BLACK is under Renju rules: no double-three, no double-four, no overline (6+ in a row).\n";
    }
    s += "Priorities: 1) win now if possible, 2) block opponent's 4-in-row, 3) block opponent's open 3, "
         "4) make your own open 3/4, 5) play near existing stones.\n\n";

    s += "Board (rows 0-14 top->bottom, cols 0-14 left->right):\n   ";
    for (int c = 0; c < BOARD; ++c) s += QString("%1 ").arg(c % 10);
    s += "\n";
    for (int r = 0; r < BOARD; ++r) {
        s += QString("%1  ").arg(r, 2);
        for (int c = 0; c < BOARD; ++c) {
            int v = board[r][c];
            s += (v == 0) ? QChar('.') : (v == 1 ? QChar('B') : QChar('W'));
            s += ' ';
        }
        s += '\n';
    }
    s += "\nRespond with ONLY your move as two numbers \"row,col\" (both 0-14). No explanation, no other text. Example: 7,7";
    return s;
}

void GameWidget::onGeminiTextReady(const QString& text)
{
    aiThinking_ = false;
    if (!playing || gameOver_ || paused_) return;
    if (aiDifficulty_ != AIDifficulty::Hard) return;

    qDebug() << "[Gemini] reply:" << text;

    QRegularExpression re(R"((\d{1,2})\s*[,\s]\s*(\d{1,2}))");
    auto m = re.match(text);
    int aiStone = (myStone == 1) ? 2 : 1;
    if (!m.hasMatch()) {
        qDebug() << "[Gemini] parse fail -> fallback Medium";
        doAiMoveMedium();
        return;
    }

    int r = m.captured(1).toInt();
    int c = m.captured(2).toInt();

    // 유효성 검증: 범위/빈칸/흑 금수 체크. 문제 있으면 Medium 로직으로 대체.
    bool bad = (r < 0 || r >= BOARD || c < 0 || c >= BOARD)
            || (board[r][c] != 0)
            || (aiStone == 1 && isForbiddenDoubleThree(r, c, aiStone));
    if (bad) {
        qDebug() << "[Gemini] invalid move (" << r << "," << c << ") -> fallback Medium";
        doAiMoveMedium();
        return;
    }

    placeStone(r, c);
}

void GameWidget::onGeminiFailed(const QString& reason)
{
    aiThinking_ = false;
    qDebug() << "[Gemini] failed:" << reason << "-> fallback Medium";
    if (!playing || gameOver_ || paused_) return;
    if (aiDifficulty_ != AIDifficulty::Hard) return;
    emit ruleViolation("⚠ Gemini Token 부족 / 보통 AI 로 대체");
    doAiMoveMedium();
}

void GameWidget::doAiMoveEasy()
{
    int aiStone = (myStone == 1) ? 2 : 1;
    int bestR = -1, bestC = -1, bestScore = -1;

    for (int r = 0; r < BOARD; ++r) {
        for (int c = 0; c < BOARD; ++c) {
            if (board[r][c]) continue;
            if (isForbiddenDoubleThree(r, c, aiStone)) continue;
            int sc = aiScore(r, c, aiStone) * 2 + aiScore(r, c, myStone);
            if (sc > bestScore) { bestScore = sc; bestR = r; bestC = c; }
        }
    }

    if (bestR == -1) {
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

void GameWidget::doAiMoveMedium()
{
    int aiStone  = (myStone == 1) ? 2 : 1;
    int oppStone = myStone;

    // 후보는 이미 놓인 돌 주변 2칸 이내로 제한 (성능 + 엉뚱한 수 방지)
    auto isCandidate = [&](int r, int c) {
        if (board[r][c]) return false;
        for (int dr = -2; dr <= 2; ++dr)
            for (int dc = -2; dc <= 2; ++dc) {
                int nr = r + dr, nc = c + dc;
                if (nr<0||nr>=BOARD||nc<0||nc>=BOARD) continue;
                if (board[nr][nc]) return true;
            }
        return false;
    };

    int bestR = -1, bestC = -1;
    long long bestScore = -1;

    for (int r = 0; r < BOARD; ++r) {
        for (int c = 0; c < BOARD; ++c) {
            if (!isCandidate(r, c)) continue;
            if (isForbiddenDoubleThree(r, c, aiStone)) continue;

            int my  = evalCellMedium(r, c, aiStone);
            int opp = evalCellMedium(r, c, oppStone);

            // 내가 5목 완성 가능한 자리면 바로 감.
            if (my >= 100000) { placeStone(r, c); return; }

            // 수비 가중치 살짝 높게 (1.1배). 상대가 여기서 5목/열린4 만드는 걸 반드시 막음.
            long long sc = (long long)my + (long long)(opp * 1.1);
            if (sc > bestScore) { bestScore = sc; bestR = r; bestC = c; }
        }
    }

    if (bestR == -1) {
        // 아무것도 없으면 중앙 근처
        int center = BOARD / 2;
        for (int d = 0; d < 5 && bestR == -1; ++d) {
            for (int dr = -d; dr <= d && bestR == -1; ++dr)
                for (int dc = -d; dc <= d && bestR == -1; ++dc) {
                    int r = center + dr, c = center + dc;
                    if (r>=0&&r<BOARD&&c>=0&&c<BOARD
                        && !board[r][c]
                        && !isForbiddenDoubleThree(r, c, aiStone)) {
                        bestR = r; bestC = c;
                    }
                }
        }
    }

    if (bestR >= 0) placeStone(bestR, bestC);
}

// (r,c) 에 stone 을 뒀을 때, 4방향 패턴 점수의 합을 반환.
int GameWidget::evalCellMedium(int r, int c, int stone) const
{
    const int dr[] = {0, 1, 1, 1};
    const int dc[] = {1, 0, 1, -1};
    int total = 0;

    const_cast<GameWidget*>(this)->board[r][c] = stone;   // 임시 배치
    for (int d = 0; d < 4; ++d) {
        QVector<int> line; line.reserve(9);
        for (int off = -4; off <= 4; ++off) {
            int nr = r + dr[d]*off, nc = c + dc[d]*off;
            if (nr<0||nr>=BOARD||nc<0||nc>=BOARD) line.append(-1);  // 벽
            else                                   line.append(board[nr][nc]);
        }
        total += evalLineMedium(line, 4, stone);
    }
    const_cast<GameWidget*>(this)->board[r][c] = 0;
    return total;
}

// 9칸 라인에서 center 위치의 stone 을 중심으로 연속돌 개수와 끝 개방성을 보고 점수화.
// line 원소: 0=빈, 1=흑, 2=백, -1=벽 (막힌 것으로 취급)
int GameWidget::evalLineMedium(const QVector<int>& line, int center, int stone) const
{
    // center 위치를 포함하는 연속 run 구간 [left, right] 찾기
    int left = center, right = center;
    while (left  > 0                 && line[left  - 1] == stone) --left;
    while (right < line.size() - 1   && line[right + 1] == stone) ++right;
    int count = right - left + 1;

    // 양쪽 끝이 빈칸이면 "열림", 벽/상대돌이면 "막힘"
    bool leftOpen  = (left  > 0                 && line[left  - 1] == 0);
    bool rightOpen = (right < line.size() - 1   && line[right + 1] == 0);

    if (count >= 5) return 100000;
    if (count == 4) {
        if (leftOpen && rightOpen) return 50000;  // 열린 4 - 다음 수 승리 확정
        if (leftOpen || rightOpen) return 1000;   // 막힌 4 - 상대가 한쪽 막으면 끝
        return 0;
    }
    if (count == 3) {
        if (leftOpen && rightOpen) return 500;    // 열린 3
        if (leftOpen || rightOpen) return 60;     // 막힌 3
        return 0;
    }
    if (count == 2) {
        if (leftOpen && rightOpen) return 50;     // 열린 2
        if (leftOpen || rightOpen) return 8;
        return 0;
    }
    // count == 1 (방금 둔 돌 하나만)
    if (leftOpen && rightOpen) return 4;
    if (leftOpen || rightOpen) return 1;
    return 0;
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
