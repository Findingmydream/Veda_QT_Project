#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QLabel>
#include <QComboBox>
#include <QTableWidget>
#include <QDateEdit>
#include <QDate>
#include "gamewidget.h"
#include "networkmanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    // ── Single-player ─────────────────────────────────
    void onStartPause();
    void onGameOver(QString name, int score, int kills, int level, int playSec);
    void onStatsChanged(int score, int lives, int level);

    // ── Records ───────────────────────────────────────
    void onSearchClicked();
    void onResetClicked();

    // ── Multi-player ──────────────────────────────────
    void onMultiConnect();
    void onMultiCancel();
    void onMultiStartGame();
    void onNetPeerConnected();
    void onNetPeerDisconnected();
    void onNetMessage(QJsonObject obj);
    void onNetError(QString err);

private:
    void setupGameTab();
    void setupRecordsTab();
    void setupMultiTab();
    void refreshRecords(const QString& name = QString(),
                        const QDate& from   = QDate(),
                        const QDate& to     = QDate());
    void startNetworkGameNow();

    QTabWidget* tabs;

    // ── Game tab ──────────────────────────────────────
    QLineEdit*   nameEdit;
    QComboBox*   difficultyCombo;
    QPushButton* startBtn;
    QLabel*      scoreLabel;
    QLabel*      livesLabel;
    QLabel*      levelLabel;
    GameWidget*  game;

    // ── Records tab ───────────────────────────────────
    QLineEdit*    searchName;
    QDateEdit*    fromDate;
    QDateEdit*    toDate;
    QTableWidget* recordTable;
    QLabel*       summaryLabel;

    // ── Multi tab ─────────────────────────────────────
    QRadioButton* hostRadio;
    QRadioButton* joinRadio;
    QLineEdit*    multiNameEdit;
    QLineEdit*    portEdit;
    QLineEdit*    ipEdit;
    QPushButton*  connectBtn;
    QPushButton*  cancelBtn;
    QLabel*       netStatusLabel;
    QLabel*       playerListLabel;
    QPushButton*  multiStartBtn;

    NetworkManager* netManager      = nullptr;
    QString         remotePlayerName;
    bool            iAmHost         = false;
};
