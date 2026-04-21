#pragma once
#include <QMainWindow>
#include <QTabWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QComboBox>
#include <QRadioButton>
#include <QRandomGenerator>
#include "gamewidget.h"
#include "networkmanager.h"
#include "database.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const Player& loggedInPlayer, QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // ── 게임 탭 ───────────────────────────────────────────────────────────────
    void onStartSingle();
    void onGameFinished(QString winner, int moves, int durationSeconds, int myStone);
    void onLogout();
    void onTogglePause();
    void onRematch();
    void onLeaveRoom();

    // ── 프로필 탭 ─────────────────────────────────────────────────────────────
    void onNickEditToggle();
    void onAvatarEditClicked();
    void onAvatarCaptureClicked();
    void onOpponentSearch();

    // ── 플레이어 조회 탭 ──────────────────────────────────────────────────────
    void onRecordSearch();
    void onShowPlayerDetail(int playerId);

    // ── 전적 탭 (내 게임 기록) ────────────────────────────────────────────────
    void onMyRecordsRefresh();
    void onDeleteMyRecord();

    // ── 멀티 탭 ───────────────────────────────────────────────────────────────
    void onMultiConnect();
    void onMultiCancel();
    void onMultiStart();
    void onNetPeerConnected();
    void onNetPeerDisconnected();
    void onNetMessage(QJsonObject);
    void onNetError(QString);

private:
    void setupStyle();
    void setupGameTab();
    void setupProfileTab();
    void setupMyRecordsTab();
    void setupRecordTab();
    void setupMultiTab();
    void startNetworkGame(int hostStone = 1);
    void tryStartRematch();
    void handleRoomLeft(const QString& message);
    void applyLoggedInPlayer(const Player& player);
    void updateHeaderStatus(const QString& text = QString());

    void refreshPlayerList();
    void refreshProfile();
    void refreshRecordTable();
    void refreshMyRecordsTable();
    void refreshOpponentStats(const QString& opponent);

    QString currentPlayerName() const;

    Ui::MainWindow* ui = nullptr;
    QTabWidget*  tabs;
    GameWidget*  game;
    Player       loggedInPlayer_;

    // ── 게임 탭 ───────────────────────────────────────────────────────────────
    QComboBox*   playerCombo;
    QLabel*      turnLabel;
    QLabel*      statusLabel;
    QRadioButton* firstRadio;

    // ── 프로필 탭 (재구성) ────────────────────────────────────────────────────
    QLabel*       profileAvatar        = nullptr;
    QPushButton*  avatarEditBtn        = nullptr;
    QPushButton*  avatarCaptureBtn     = nullptr;
    QLabel*       profileNickLabel     = nullptr;
    QLineEdit*    profileNickEdit      = nullptr;   // 인라인 편집용
    QPushButton*  nickEditBtn          = nullptr;
    QLabel*       profileTitleLabel    = nullptr;

    // 내 전적 sub-tab
    QLabel*       profileTotalLabel    = nullptr;
    QLabel*       profileWinRateLabel  = nullptr;
    QLabel*       profileAvgMovesLabel = nullptr;
    QLabel*       profileWLDLabel      = nullptr;
    QTableWidget* myRecentTable        = nullptr;

    // 상대 전적 sub-tab
    QPushButton*  opponentSearchBtn    = nullptr;
    QLabel*       opponentNameLabel    = nullptr;
    QLabel*       opponentTotalLabel   = nullptr;
    QLabel*       opponentWLDLabel     = nullptr;
    QLabel*       opponentWinRateLabel = nullptr;
    QTableWidget* opponentRecordTable  = nullptr;
    QString       selectedOpponent_;

    int           selectedPlayerId     = -1;

    // ── 플레이어 조회 탭 ──────────────────────────────────────────────────────
    QLineEdit*    recordSearchEdit      = nullptr;
    QComboBox*    recordSortCombo       = nullptr;
    QTableWidget* recordPlayerListTable = nullptr;

    // ── 전적 탭 (내 게임 기록) ────────────────────────────────────────────────
    QComboBox*    myRecordsResultCombo = nullptr;
    QTableWidget* myRecordsTable       = nullptr;
    QPushButton*  myRecordsDeleteBtn   = nullptr;

    // ── 멀티 탭 ───────────────────────────────────────────────────────────────
    QRadioButton* hostRadio;
    QRadioButton* joinRadio;
    QComboBox*    multiPlayerCombo;
    QLineEdit*    portEdit;
    QLineEdit*    ipEdit;
    QPushButton*  connectBtn;
    QPushButton*  cancelBtn;
    QPushButton*  multiStartBtn;
    QLabel*       netStatusLabel;
    QLabel*       playerListLabel;

    NetworkManager* netManager = nullptr;
    bool iAmHost = false;
    bool localRematchRequested = false;
    bool remoteRematchRequested = false;
    QString remotePlayerName;
    QString currentOpponentName = "AI";
};
