#pragma once
#include <QWidget>
#include <QVector>
#include <QJsonObject>
#include <QDateTime>
#include <QPoint>

class NetworkManager;
class QTimer;

class GameWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GameWidget(QWidget* parent = nullptr);

    // 싱글 (vs AI)
    void startSingle(const QString& playerName, bool playerFirst = true);

    // 멀티
    void startNetwork(int myStoneColor, NetworkManager* net,
                      const QString& myName, const QString& opName);

    bool isPlaying() const;
    void resetToIdle();
    bool isPaused() const;
    void setPaused(bool paused);

signals:
    void gameFinished(QString winner, int moves, int durationSeconds, int myStone); // 게임 끝날 때
    void ruleViolation(QString message);
    void turnTimeChanged(int secondsLeft, bool urgent);
    void turnTimedOut(QString message);

public slots:
    void handleNetMessage(const QJsonObject& obj);

protected:
    void paintEvent(QPaintEvent*)        override;
    void mousePressEvent(QMouseEvent*)   override;
    QSize sizeHint() const               override;

private:
    void   resetBoard();
    bool   checkWin(int r, int c, int stone) const;
    void   doAiMove();
    int    aiScore(int r, int c, int stone) const;
    bool   placeStone(int r, int c);     // 공용 착수 처리
    void   endGame(int winner);          // 0=무 1=흑 2=백
    bool   isForbiddenDoubleThree(int r, int c, int stone);
    int    countOpenThreesAt(int r, int c) const;
    bool   isOpenThreeLine(const QVector<int>& line, int center) const;
    void   refreshForbiddenPreview();
    void   resetTurnTimer();
    void   stopTurnTimer();
    void   onTurnTimerTick();

    // ── 보드 상태 ─────────────────────────────────────────────────────────────
    static constexpr int BOARD = 15;    // 15x15
    static constexpr int CELL  = 36;    // 셀 크기 px
    static constexpr int MARGIN = 24;

    int board[BOARD][BOARD] = {};       // 0=빈 1=흑 2=백
    int currentStone = 1;               // 현재 차례 돌 (1=흑 2=백)
    int moveCount    = 0;
    bool playing     = false;
    bool gameOver_   = false;
    bool paused_     = false;
    int  secondsLeft_ = 30;
    QTimer* turnTimer_ = nullptr;
    QString winner_;

    // ── 플레이어 정보 ─────────────────────────────────────────────────────────
    QString myName_;
    QString opName_;
    int     myStone  = 1;   // 내 돌 색 (1=흑 2=백)
    bool    myTurn   = true;

    // ── 네트워크 ──────────────────────────────────────────────────────────────
    NetworkManager* net_    = nullptr;
    bool            netMode = false;

    // ── 마지막 착수 위치 (강조 표시용) ───────────────────────────────────────
    int lastR = -1, lastC = -1;
    int forbiddenR = -1, forbiddenC = -1;

    // 멀티 플레이 중 흑 차례에 모든 33 금수 자리를 미리 보여주기 위한 목록.
    QVector<QPoint> forbiddenPreview_;

    QDateTime gameStartTime_;
};
