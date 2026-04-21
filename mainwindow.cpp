#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QJsonObject>
#include <QNetworkInterface>
#include <QRandomGenerator>
#include <QTimer>
#include <QMap>
#include <QPixmap>
#include <QFileInfo>
#include <QFileDialog>
#include <QSet>
#include <QTabWidget>
#include <algorithm>
#include "logindialog.h"
#include "profiledialog.h"
#include "playersearchdialog.h"
#include "cameracapturedialog.h"
#include "playerdetaildialog.h"
#include "titles.h"

// ─────────────────────────────────────────────────────────────────────────────
MainWindow::MainWindow(const Player& loggedInPlayer, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , loggedInPlayer_(loggedInPlayer)
{
    Database::instance().init();
    ui->setupUi(this);

    tabs = ui->tabs;
    game = ui->game;
    playerCombo = ui->playerCombo;
    firstRadio = ui->firstRadio;
    statusLabel = ui->statusLabel;
    hostRadio = ui->hostRadio;
    joinRadio = ui->joinRadio;
    multiPlayerCombo = ui->multiPlayerCombo;
    portEdit = ui->portEdit;
    ipEdit = ui->ipEdit;
    connectBtn = ui->connectBtn;
    cancelBtn = ui->cancelBtn;
    multiStartBtn = ui->multiStartBtn;
    netStatusLabel = ui->netStatusLabel;
    playerListLabel = ui->playerListLabel;

    // 기존 ui 의 프로필/전적 탭은 통째로 빼고 setup 함수 안에서 새 위젯으로 대체.
    int profileIdx = tabs->indexOf(ui->profileTab);
    if (profileIdx >= 0) tabs->removeTab(profileIdx);
    int recordIdx = tabs->indexOf(ui->recordTab);
    if (recordIdx >= 0) tabs->removeTab(recordIdx);

    setupStyle();
    setupGameTab();
    setupProfileTab();    // 1번 위치
    setupMyRecordsTab();  // 2번 위치 (전적)
    setupRecordTab();     // 3번 위치 (플레이어 조회)
    setupMultiTab();

    setWindowTitle(QString("⚫⚪  오목 게임 - %1").arg(loggedInPlayer_.nickname));
    ui->accountLabel->setText("로그인: " + loggedInPlayer_.nickname);

    connect(tabs, &QTabWidget::currentChanged, [this](int i) {
        if (i == 1) refreshPlayerList();                                   // 내 프로필
        if (i == 2) { refreshPlayerList(); refreshMyRecordsTable(); }      // 전적
        if (i == 3) { refreshPlayerList(); refreshRecordTable(); }         // 플레이어 조회
        if (i == 4)   refreshPlayerList();                                 // 멀티
        updateHeaderStatus();
    });

    refreshPlayerList();
    updateHeaderStatus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupStyle()
{
    // Visual styling is stored in mainwindow.ui so the screen can be edited in Qt Designer.
}

// ─────────────────────────────────────────────────────────────────────────────
// 게임 탭
void MainWindow::setupGameTab()
{
    firstRadio->setChecked(true);
    connect(ui->startSingleBtn, &QPushButton::clicked, this, &MainWindow::onStartSingle);
    connect(ui->pauseBtn, &QPushButton::clicked, this, &MainWindow::onTogglePause);
    connect(ui->rematchBtn, &QPushButton::clicked, this, &MainWindow::onRematch);
    connect(ui->leaveRoomBtn, &QPushButton::clicked, this, &MainWindow::onLeaveRoom);
    connect(ui->logoutBtn, &QPushButton::clicked, this, &MainWindow::onLogout);
    connect(game, &GameWidget::gameFinished, this, &MainWindow::onGameFinished);
    connect(game, &GameWidget::ruleViolation, [this](const QString& message) {
        statusLabel->setText(message);
        updateHeaderStatus(message);
    });
    connect(game, &GameWidget::turnTimeChanged, [this](int secondsLeft, bool urgent) {
        ui->turnTimerLabel->setText(QString("%1초").arg(secondsLeft));
        ui->turnTimerLabel->setStyleSheet(urgent
            ? "color:#ff4040; font-size:16px; font-weight:bold; padding:3px 8px;"
            : "color:#00ff88; font-size:16px; font-weight:bold; padding:3px 8px;");
    });
    connect(game, &GameWidget::turnTimedOut, [this](const QString& message) {
        statusLabel->setText(message);
        updateHeaderStatus(message);
    });
    ui->rematchBtn->setVisible(false);
    ui->leaveRoomBtn->setVisible(false);
}

void MainWindow::onStartSingle()
{
    QString name = currentPlayerName();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "알림", "먼저 프로필 탭에서 플레이어를 만드세요.");
        return;
    }
    bool first = firstRadio->isChecked();
    currentOpponentName = "AI";
    localRematchRequested = false;
    remoteRematchRequested = false;
    game->startSingle(name, first);
    ui->pauseBtn->setEnabled(true);
    ui->pauseBtn->setText("⏸  일시정지");
    ui->rematchBtn->setVisible(false);
    ui->leaveRoomBtn->setVisible(false);
    updateHeaderStatus(QString("AI 대전 중 - %1 선공").arg(first ? name : "AI"));
    statusLabel->setText(QString("%1 vs AI  |  %2 선공")
        .arg(name, first ? name : "AI"));
}

void MainWindow::onGameFinished(QString winner, int moves)
{
    QString name = currentPlayerName();
    Player p = Database::instance().readPlayerByName(name);
    if (p.id <= 0) return;

    QString result;
    if (winner.isEmpty())   result = "무";
    else if (winner == name) result = "승";
    else                     result = "패";

    QString opponent = currentOpponentName.isEmpty() ? "AI" : currentOpponentName;
    Database::instance().createRecord(p.id, opponent, result, moves);
    ui->pauseBtn->setEnabled(false);
    ui->pauseBtn->setText("⏸  일시정지");
    const bool multiGame = currentOpponentName.compare("AI", Qt::CaseInsensitive) != 0
        && netManager && netManager->isConnected();
    ui->rematchBtn->setVisible(multiGame);
    ui->rematchBtn->setEnabled(multiGame);
    ui->rematchBtn->setText("↻  재도전");
    ui->leaveRoomBtn->setVisible(multiGame);
    ui->leaveRoomBtn->setEnabled(multiGame);
    updateHeaderStatus(winner.isEmpty() ? "게임 종료 - 무승부" : "게임 종료 - " + winner + " 승리");
    statusLabel->setText(QString("게임 종료! %1  |  %2수")
        .arg(winner.isEmpty() ? "무승부" : winner + " 승리").arg(moves));
}

void MainWindow::onRematch()
{
    if (!netManager || !netManager->isConnected()) return;

    localRematchRequested = true;
    ui->rematchBtn->setEnabled(false);
    ui->rematchBtn->setText("상대 대기 중");
    statusLabel->setText("재도전 요청을 보냈습니다.");
    updateHeaderStatus("재도전 대기 중");

    QJsonObject msg;
    msg["t"] = "rematch";
    netManager->sendJson(msg);
    tryStartRematch();
}

void MainWindow::onLeaveRoom()
{
    if (netManager && netManager->isConnected()) {
        QJsonObject msg;
        msg["t"] = "leave";
        netManager->sendJson(msg);
    }
    handleRoomLeft("방을 나갔습니다.");
}

void MainWindow::onTogglePause()
{
    if (!game->isPlaying()) return;

    bool pause = !game->isPaused();
    game->setPaused(pause);
    ui->pauseBtn->setText(pause ? "▶  계속하기" : "⏸  일시정지");
    bool multiGame = currentOpponentName.compare("AI", Qt::CaseInsensitive) != 0;
    QString gameName = multiGame ? "멀티 대전" : "AI 대전";
    updateHeaderStatus(pause ? gameName + " 일시정지" : gameName + " 중");
    statusLabel->setText(pause ? "게임이 일시정지되었습니다." : "게임을 다시 시작했습니다.");
}

void MainWindow::onLogout()
{
    if (game && game->isPlaying()) {
        if (QMessageBox::question(this, "로그아웃", "진행 중인 게임을 종료하고 로그아웃할까요?")
            != QMessageBox::Yes) {
            return;
        }
    }

    if (netManager) {
        netManager->closeAll();
        netManager->deleteLater();
        netManager = nullptr;
    }

    hide();
    LoginDialog login;
    if (login.exec() == QDialog::Accepted) {
        applyLoggedInPlayer(login.loggedInPlayer());
        show();
        tabs->setCurrentIndex(0);
    } else {
        close();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 내 프로필 탭 (재구성)
void MainWindow::setupProfileTab()
{
    auto* w = new QWidget;
    auto* root = new QVBoxLayout(w);
    root->setContentsMargins(20, 20, 20, 20);
    root->setSpacing(15);

    // === 헤더: 사진 + 닉네임/칭호 + 인라인 편집 ===
    auto* headerRow = new QHBoxLayout;
    headerRow->setSpacing(20);

    // 좌: 사진 + 사진 변경 버튼
    auto* avatarCol = new QVBoxLayout;
    avatarCol->setSpacing(6);
    profileAvatar = new QLabel;
    profileAvatar->setFixedSize(180, 180);
    profileAvatar->setAlignment(Qt::AlignCenter);
    profileAvatar->setStyleSheet("border:2px solid #2a2a4a; background:#0d1117; color:#888; border-radius:8px;");
    avatarCol->addWidget(profileAvatar, 0, Qt::AlignHCenter);

    auto* avatarBtnRow = new QHBoxLayout;
    avatarBtnRow->setSpacing(4);
    avatarEditBtn    = new QPushButton("📷  사진 변경");
    avatarCaptureBtn = new QPushButton("📸  사진 생성");
    avatarBtnRow->addWidget(avatarEditBtn);
    avatarBtnRow->addWidget(avatarCaptureBtn);
    avatarCol->addLayout(avatarBtnRow);
    headerRow->addLayout(avatarCol, 0);

    // 우: 닉네임 + 칭호
    auto* infoCol = new QVBoxLayout;
    infoCol->setSpacing(8);

    auto* nickRow = new QHBoxLayout;
    profileNickLabel = new QLabel;
    profileNickLabel->setStyleSheet("color:white; font-size:22px; font-weight:bold;");
    profileNickEdit = new QLineEdit;
    profileNickEdit->setStyleSheet("color:white; font-size:18px; font-weight:bold; padding:2px 6px;");
    profileNickEdit->hide();
    nickEditBtn = new QPushButton("✏️");
    nickEditBtn->setMaximumWidth(36);
    nickEditBtn->setToolTip("닉네임 변경");
    nickRow->addWidget(profileNickLabel);
    nickRow->addWidget(profileNickEdit, 1);
    nickRow->addWidget(nickEditBtn);
    nickRow->addStretch(1);
    infoCol->addLayout(nickRow);

    profileTitleLabel = new QLabel;
    profileTitleLabel->setStyleSheet("color:#ffcc66; font-size:15px;");
    infoCol->addWidget(profileTitleLabel);

    // 헤더 우측에 전적 요약 박스 (프로필 정보 옆)
    auto* headerStatsBox = new QGroupBox("전적 요약");
    auto* headerStatsForm = new QFormLayout(headerStatsBox);
    profileTotalLabel    = new QLabel("0판");
    profileWLDLabel      = new QLabel("0승 0패 0무");
    profileWinRateLabel  = new QLabel("0.0%");
    profileAvgMovesLabel = new QLabel("-");
    headerStatsForm->addRow("총 게임 수:",  profileTotalLabel);
    headerStatsForm->addRow("승 / 패 / 무:", profileWLDLabel);
    headerStatsForm->addRow("승률:",        profileWinRateLabel);
    headerStatsForm->addRow("평균 수:",     profileAvgMovesLabel);
    infoCol->addWidget(headerStatsBox);
    infoCol->addStretch(1);

    headerRow->addLayout(infoCol, 1);
    root->addLayout(headerRow);

    // === 전적 sub-tabs ===
    auto* statsTabs = new QTabWidget;

    // --- Tab 1: 내 전적 (기록 테이블만) ---
    auto* myTab = new QWidget;
    auto* myLay = new QVBoxLayout(myTab);
    myRecentTable = new QTableWidget;
    myRecentTable->setColumnCount(4);
    myRecentTable->setHorizontalHeaderLabels({"상대","결과","수","날짜"});
    myRecentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    myRecentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    myRecentTable->setAlternatingRowColors(true);
    myRecentTable->verticalHeader()->setVisible(false);
    myRecentTable->horizontalHeader()->setStretchLastSection(true);
    myLay->addWidget(myRecentTable, 1);
    statsTabs->addTab(myTab, "내 전적");

    // --- Tab 2: 상대 전적 ---
    auto* oppTab = new QWidget;
    auto* oppLay = new QVBoxLayout(oppTab);

    auto* oppTopRow = new QHBoxLayout;
    opponentSearchBtn = new QPushButton("🔍  상대 검색");
    opponentNameLabel = new QLabel("(선택 안됨)");
    opponentNameLabel->setStyleSheet("color:#cfd8ff; font-weight:bold; padding:0 8px;");
    oppTopRow->addWidget(opponentSearchBtn);
    oppTopRow->addWidget(opponentNameLabel);
    oppTopRow->addStretch(1);
    oppLay->addLayout(oppTopRow);

    auto* oppStatsBox = new QGroupBox("상대 전적 (head-to-head)");
    auto* oppStatsForm = new QFormLayout(oppStatsBox);
    opponentTotalLabel   = new QLabel("0판");
    opponentWLDLabel     = new QLabel("0승 0패 0무");
    opponentWinRateLabel = new QLabel("0.0%");
    oppStatsForm->addRow("총 게임 수:",  opponentTotalLabel);
    oppStatsForm->addRow("승 / 패 / 무:", opponentWLDLabel);
    oppStatsForm->addRow("승률:",        opponentWinRateLabel);
    oppLay->addWidget(oppStatsBox);

    opponentRecordTable = new QTableWidget;
    opponentRecordTable->setColumnCount(3);
    opponentRecordTable->setHorizontalHeaderLabels({"결과","수","날짜"});
    opponentRecordTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    opponentRecordTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    opponentRecordTable->setAlternatingRowColors(true);
    opponentRecordTable->verticalHeader()->setVisible(false);
    opponentRecordTable->horizontalHeader()->setStretchLastSection(true);
    oppLay->addWidget(opponentRecordTable, 1);

    statsTabs->addTab(oppTab, "상대 전적");

    root->addWidget(statsTabs, 1);

    tabs->insertTab(1, w, "👤  내 프로필");

    connect(avatarEditBtn,     &QPushButton::clicked, this, &MainWindow::onAvatarEditClicked);
    connect(avatarCaptureBtn,  &QPushButton::clicked, this, &MainWindow::onAvatarCaptureClicked);
    connect(nickEditBtn,       &QPushButton::clicked, this, &MainWindow::onNickEditToggle);
    connect(profileNickEdit,   &QLineEdit::returnPressed, this, &MainWindow::onNickEditToggle);
    connect(opponentSearchBtn, &QPushButton::clicked, this, &MainWindow::onOpponentSearch);
}

void MainWindow::onAvatarEditClicked()
{
    if (loggedInPlayer_.id <= 0) return;
    QString path = QFileDialog::getOpenFileName(
        this, "프로필 사진 선택", QString(),
        "이미지 (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (path.isEmpty()) return;
    Database::instance().setPlayerAvatar(loggedInPlayer_.id, path);
    loggedInPlayer_ = Database::instance().readPlayer(loggedInPlayer_.id);
    refreshProfile();
}

void MainWindow::onAvatarCaptureClicked()
{
    if (loggedInPlayer_.id <= 0) return;
    CameraCaptureDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    QString path = dlg.savedPath();
    if (path.isEmpty()) return;
    Database::instance().setPlayerAvatar(loggedInPlayer_.id, path);
    loggedInPlayer_ = Database::instance().readPlayer(loggedInPlayer_.id);
    refreshProfile();
}

void MainWindow::onNickEditToggle()
{
    if (loggedInPlayer_.id <= 0) return;

    if (profileNickEdit->isVisible()) {
        // 저장 단계
        QString newNick = profileNickEdit->text().trimmed();
        if (newNick.isEmpty()) {
            QMessageBox::warning(this, "알림", "닉네임을 입력하세요.");
            return;
        }
        if (newNick != loggedInPlayer_.nickname &&
            Database::instance().playerExists(newNick)) {
            QMessageBox::warning(this, "알림", "이미 사용 중인 닉네임입니다.");
            return;
        }
        if (newNick != loggedInPlayer_.nickname) {
            if (!Database::instance().updatePlayer(loggedInPlayer_.id, newNick, QString())) {
                QMessageBox::warning(this, "알림", "닉네임 변경에 실패했습니다.");
                return;
            }
            loggedInPlayer_ = Database::instance().readPlayer(loggedInPlayer_.id);
            setWindowTitle(QString("⚫⚪  오목 게임 - %1").arg(loggedInPlayer_.nickname));
            ui->accountLabel->setText("로그인: " + loggedInPlayer_.nickname);
        }
        // 뷰 모드로 복귀
        profileNickEdit->hide();
        profileNickLabel->show();
        nickEditBtn->setText("✏️");
        refreshPlayerList();
    } else {
        // 편집 모드 진입
        profileNickEdit->setText(loggedInPlayer_.nickname);
        profileNickLabel->hide();
        profileNickEdit->show();
        profileNickEdit->setFocus();
        profileNickEdit->selectAll();
        nickEditBtn->setText("💾");
    }
}

void MainWindow::onOpponentSearch()
{
    if (loggedInPlayer_.id <= 0) return;

    auto records = Database::instance().readRecords(loggedInPlayer_.id);
    QSet<QString> oppsSet;
    for (const auto& r : records) {
        QString op = r.opponent.trimmed();
        if (!op.isEmpty()) oppsSet.insert(op);
    }
    QStringList opps(oppsSet.begin(), oppsSet.end());
    opps.sort();

    if (opps.isEmpty()) {
        QMessageBox::information(this, "알림", "아직 대전 기록이 없습니다.");
        return;
    }

    PlayerSearchDialog dlg(opps, this);
    if (dlg.exec() != QDialog::Accepted) return;
    selectedOpponent_ = dlg.selectedName();
    opponentNameLabel->setText(selectedOpponent_);
    refreshOpponentStats(selectedOpponent_);
}

void MainWindow::refreshPlayerList()
{
    loggedInPlayer_ = Database::instance().readPlayer(loggedInPlayer_.id);

    playerCombo->clear();
    multiPlayerCombo->clear();

    if (loggedInPlayer_.id > 0) {
        playerCombo->addItem(loggedInPlayer_.nickname);
        multiPlayerCombo->addItem(loggedInPlayer_.nickname);
        selectedPlayerId = loggedInPlayer_.id;
    }

    refreshProfile();
}

void MainWindow::refreshProfile()
{
    if (!profileNickLabel) return;
    const Player& p = loggedInPlayer_;

    profileNickLabel->setText(p.nickname.isEmpty() ? "(로그인 안됨)" : p.nickname);
    profileTitleLabel->setText(playerTitle(p.totalGames(), p.winRate()));
    profileTotalLabel->setText(QString::number(p.totalGames()) + "판");
    profileWLDLabel->setText(QString("%1승 %2패 %3무").arg(p.wins).arg(p.losses).arg(p.draws));
    profileWinRateLabel->setText(QString::number(p.winRate(), 'f', 1) + "%");

    double avg = Database::instance().averageMoves(p.id);
    profileAvgMovesLabel->setText(p.totalGames() > 0
        ? QString::number(avg, 'f', 1) + "수"
        : "-");

    // 사진
    bool avatarSet = false;
    if (!p.avatarPath.isEmpty() && QFileInfo::exists(p.avatarPath)) {
        QPixmap pix(p.avatarPath);
        if (!pix.isNull()) {
            profileAvatar->setPixmap(pix.scaled(profileAvatar->size(),
                                                Qt::KeepAspectRatio,
                                                Qt::SmoothTransformation));
            avatarSet = true;
        }
    }
    if (!avatarSet) {
        profileAvatar->setPixmap(QPixmap());
        profileAvatar->setText("(사진 없음)");
    }

    // 내 최근 기록 테이블
    if (myRecentTable) {
        auto records = Database::instance().readRecords(p.id);
        myRecentTable->setRowCount(records.size());
        for (int i = 0; i < records.size(); ++i) {
            const auto& r = records[i];
            auto setC = [&](int col, const QString& t, QColor color = Qt::white) {
                auto* it = new QTableWidgetItem(t);
                it->setTextAlignment(Qt::AlignCenter);
                it->setForeground(color);
                myRecentTable->setItem(i, col, it);
            };
            QColor rc = (r.result=="승") ? QColor(100,220,100)
                      : (r.result=="패") ? QColor(220,100,100)
                      : QColor(200,200,100);
            QString opp = (r.opponent.compare("AI", Qt::CaseInsensitive) == 0)
                        ? "AI" : r.opponent;
            setC(0, opp);
            setC(1, r.result, rc);
            setC(2, QString::number(r.moves));
            setC(3, r.playedAt.toString("MM-dd hh:mm"));
        }
    }

    // 상대 전적도 함께 갱신 (선택된 상대가 있으면)
    if (!selectedOpponent_.isEmpty())
        refreshOpponentStats(selectedOpponent_);
}

void MainWindow::refreshOpponentStats(const QString& opponent)
{
    if (!opponentTotalLabel) return;
    if (loggedInPlayer_.id <= 0 || opponent.isEmpty()) {
        opponentTotalLabel->setText("0판");
        opponentWLDLabel->setText("0승 0패 0무");
        opponentWinRateLabel->setText("0.0%");
        if (opponentRecordTable) opponentRecordTable->setRowCount(0);
        return;
    }

    auto records = Database::instance().readRecords(loggedInPlayer_.id);
    QVector<GameRecord> matched;
    int wins = 0, losses = 0, draws = 0;
    for (const auto& r : records) {
        if (r.opponent.compare(opponent, Qt::CaseInsensitive) != 0) continue;
        matched.append(r);
        if      (r.result == "승") ++wins;
        else if (r.result == "패") ++losses;
        else                        ++draws;
    }
    int total = matched.size();
    double rate = total > 0 ? (wins * 100.0 / total) : 0.0;

    opponentTotalLabel->setText(QString::number(total) + "판");
    opponentWLDLabel->setText(QString("%1승 %2패 %3무").arg(wins).arg(losses).arg(draws));
    opponentWinRateLabel->setText(QString::number(rate, 'f', 1) + "%");

    opponentRecordTable->setRowCount(matched.size());
    for (int i = 0; i < matched.size(); ++i) {
        const auto& r = matched[i];
        auto setC = [&](int col, const QString& t, QColor color = Qt::white) {
            auto* it = new QTableWidgetItem(t);
            it->setTextAlignment(Qt::AlignCenter);
            it->setForeground(color);
            opponentRecordTable->setItem(i, col, it);
        };
        QColor rc = (r.result=="승") ? QColor(100,220,100)
                  : (r.result=="패") ? QColor(220,100,100)
                  : QColor(200,200,100);
        setC(0, r.result, rc);
        setC(1, QString::number(r.moves));
        setC(2, r.playedAt.toString("MM-dd hh:mm"));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 플레이어 조회 탭 (재구성 - 상세는 다이얼로그로)
void MainWindow::setupRecordTab()
{
    auto* w = new QWidget;
    auto* root = new QVBoxLayout(w);
    root->setContentsMargins(14, 14, 14, 14);
    root->setSpacing(10);

    auto* topRow = new QHBoxLayout;
    recordSearchEdit = new QLineEdit;
    recordSearchEdit->setPlaceholderText("닉네임으로 검색...");
    recordSortCombo = new QComboBox;
    recordSortCombo->addItems({"승률순", "칭호순"});
    auto* searchBtn = new QPushButton("🔍  검색");
    topRow->addWidget(new QLabel("검색:"));
    topRow->addWidget(recordSearchEdit, 1);
    topRow->addWidget(new QLabel("정렬:"));
    topRow->addWidget(recordSortCombo);
    topRow->addWidget(searchBtn);
    root->addLayout(topRow);

    recordPlayerListTable = new QTableWidget;
    recordPlayerListTable->setColumnCount(5);
    recordPlayerListTable->setHorizontalHeaderLabels({"", "닉네임", "칭호", "총판", "승률"});
    recordPlayerListTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    recordPlayerListTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    recordPlayerListTable->setAlternatingRowColors(true);
    recordPlayerListTable->verticalHeader()->setVisible(false);
    recordPlayerListTable->horizontalHeader()->setStretchLastSection(true);
    root->addWidget(recordPlayerListTable, 1);

    tabs->insertTab(3, w, "🔍  플레이어 조회");

    connect(searchBtn,        &QPushButton::clicked, this, &MainWindow::onRecordSearch);
    connect(recordSearchEdit, &QLineEdit::returnPressed, this, &MainWindow::onRecordSearch);
    connect(recordSortCombo,  QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int) { refreshRecordTable(); });
}

// ─────────────────────────────────────────────────────────────────────────────
// 전적 탭 (내 게임 기록)
void MainWindow::setupMyRecordsTab()
{
    auto* w = new QWidget;
    auto* root = new QVBoxLayout(w);
    root->setContentsMargins(14, 14, 14, 14);
    root->setSpacing(10);

    auto* topRow = new QHBoxLayout;
    myRecordsResultCombo = new QComboBox;
    myRecordsResultCombo->addItems({"전체","승","패","무"});
    auto* refreshBtn = new QPushButton("🔄  새로고침");
    myRecordsDeleteBtn = new QPushButton("🗑  선택 삭제");
    topRow->addWidget(new QLabel("결과:"));
    topRow->addWidget(myRecordsResultCombo);
    topRow->addStretch(1);
    topRow->addWidget(refreshBtn);
    topRow->addWidget(myRecordsDeleteBtn);
    root->addLayout(topRow);

    myRecordsTable = new QTableWidget;
    myRecordsTable->setColumnCount(5);
    myRecordsTable->setHorizontalHeaderLabels({"ID","상대","결과","수","날짜"});
    myRecordsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    myRecordsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    myRecordsTable->setAlternatingRowColors(true);
    myRecordsTable->verticalHeader()->setVisible(false);
    myRecordsTable->horizontalHeader()->setStretchLastSection(true);
    myRecordsTable->setColumnHidden(0, true);  // ID는 내부용
    root->addWidget(myRecordsTable, 1);

    tabs->insertTab(2, w, "📊  전적");

    connect(refreshBtn,           &QPushButton::clicked, this, &MainWindow::onMyRecordsRefresh);
    connect(myRecordsResultCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int) { refreshMyRecordsTable(); });
    connect(myRecordsDeleteBtn,   &QPushButton::clicked, this, &MainWindow::onDeleteMyRecord);
}

void MainWindow::onRecordSearch()
{
    refreshRecordTable();
}

void MainWindow::refreshRecordTable()
{
    if (!recordPlayerListTable) return;

    QString needle = recordSearchEdit ? recordSearchEdit->text().trimmed() : QString();
    auto allPlayers = Database::instance().readAllPlayers();

    QVector<Player> filtered;
    for (const auto& p : allPlayers) {
        if (needle.isEmpty() || p.nickname.contains(needle, Qt::CaseInsensitive))
            filtered.append(p);
    }

    int sortMode = recordSortCombo ? recordSortCombo->currentIndex() : 0;
    std::sort(filtered.begin(), filtered.end(),
        [sortMode](const Player& a, const Player& b) {
            if (sortMode == 1) {
                int ra = titleRank(a.totalGames(), a.winRate());
                int rb = titleRank(b.totalGames(), b.winRate());
                if (ra != rb) return ra > rb;
            }
            return a.winRate() > b.winRate();
        });

    static const char* detailBtnQss = R"(
        QPushButton {
            background:#0f3460;
            color:white;
            border:none;
            padding:4px 10px;
            border-radius:4px;
            font-weight:bold;
        }
        QPushButton:hover {
            background:#e94560;
            color:white;
        }
        QPushButton:pressed {
            background:#b03040;
        }
    )";

    recordPlayerListTable->setRowCount(filtered.size());
    for (int i = 0; i < filtered.size(); ++i) {
        const auto& p = filtered[i];
        int pid = p.id;

        // 0번 컬럼: 상세 버튼
        auto* btn = new QPushButton("📋  상세");
        btn->setStyleSheet(detailBtnQss);
        btn->setCursor(Qt::PointingHandCursor);
        connect(btn, &QPushButton::clicked, this, [this, pid]() { onShowPlayerDetail(pid); });
        recordPlayerListTable->setCellWidget(i, 0, btn);

        auto setC = [&](int col, const QString& t) {
            auto* it = new QTableWidgetItem(t);
            it->setTextAlignment(Qt::AlignCenter);
            recordPlayerListTable->setItem(i, col, it);
        };
        setC(1, p.nickname);
        setC(2, playerTitle(p.totalGames(), p.winRate()));
        setC(3, QString::number(p.totalGames()));
        setC(4, QString::number(p.winRate(), 'f', 1) + "%");
    }
    recordPlayerListTable->setColumnWidth(0, 90);
    recordPlayerListTable->resizeColumnToContents(2);
    recordPlayerListTable->resizeColumnToContents(3);
    recordPlayerListTable->resizeColumnToContents(4);
    recordPlayerListTable->horizontalHeader()->setStretchLastSection(false);
    recordPlayerListTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
}

void MainWindow::onShowPlayerDetail(int playerId)
{
    Player p = Database::instance().readPlayer(playerId);
    if (p.id <= 0) return;
    PlayerDetailDialog dlg(p, this);
    dlg.exec();
}

// ── 전적 탭 슬롯 ──────────────────────────────────────────────────────────────
void MainWindow::onMyRecordsRefresh()
{
    refreshMyRecordsTable();
}

void MainWindow::refreshMyRecordsTable()
{
    if (!myRecordsTable) return;
    if (loggedInPlayer_.id <= 0) {
        myRecordsTable->setRowCount(0);
        return;
    }

    QString resultFilter = myRecordsResultCombo ? myRecordsResultCombo->currentText() : QString();
    if (resultFilter == "전체") resultFilter.clear();

    auto records = Database::instance().readRecords(loggedInPlayer_.id, resultFilter);
    myRecordsTable->setRowCount(records.size());
    for (int i = 0; i < records.size(); ++i) {
        const auto& r = records[i];
        auto setC = [&](int col, const QString& t, QColor color = Qt::white) {
            auto* it = new QTableWidgetItem(t);
            it->setTextAlignment(Qt::AlignCenter);
            it->setForeground(color);
            myRecordsTable->setItem(i, col, it);
        };
        QColor rc = (r.result=="승") ? QColor(100,220,100)
                  : (r.result=="패") ? QColor(220,100,100)
                  : QColor(200,200,100);
        QString opp = (r.opponent.compare("AI", Qt::CaseInsensitive) == 0)
                    ? "AI 대전" : "멀티 - " + r.opponent;
        setC(0, QString::number(r.id));
        setC(1, opp);
        setC(2, r.result, rc);
        setC(3, QString::number(r.moves));
        setC(4, r.playedAt.toString("yyyy-MM-dd hh:mm"));
    }
    myRecordsTable->resizeColumnsToContents();
    myRecordsTable->horizontalHeader()->setStretchLastSection(true);
}

void MainWindow::onDeleteMyRecord()
{
    if (!myRecordsTable) return;
    int row = myRecordsTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "알림", "삭제할 기록을 선택하세요.");
        return;
    }
    int id = myRecordsTable->item(row, 0)->text().toInt();
    if (QMessageBox::question(this, "확인", "이 기록을 삭제할까요?")
        != QMessageBox::Yes) return;
    if (!Database::instance().deleteRecord(id)) {
        QMessageBox::warning(this, "알림", "기록 삭제에 실패했습니다.");
        return;
    }
    refreshPlayerList();
    refreshMyRecordsTable();
}

// 멀티 탭
void MainWindow::setupMultiTab()
{
    hostRadio->setChecked(true);

    // 내 IP 표시
    QString localIp = "알 수 없음";
    for (const QNetworkInterface& iface : QNetworkInterface::allInterfaces()) {
        if (iface.flags().testFlag(QNetworkInterface::IsLoopBack)) continue;
        if (!iface.flags().testFlag(QNetworkInterface::IsUp))      continue;
        for (const QNetworkAddressEntry& entry : iface.addressEntries()) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                localIp = entry.ip().toString(); break;
            }
        }
        if (localIp != "알 수 없음") break;
    }
    ui->myIpEdit->setText(localIp);

    connect(hostRadio,    &QRadioButton::toggled, [this](bool host) {
        connectBtn->setText(host ? "🌐  방 만들기" : "🔌  방 참가");
        ipEdit->setEnabled(!host);
    });
    connect(connectBtn,   &QPushButton::clicked, this, &MainWindow::onMultiConnect);
    connect(cancelBtn,    &QPushButton::clicked, this, &MainWindow::onMultiCancel);
    connect(multiStartBtn,&QPushButton::clicked, this, &MainWindow::onMultiStart);
}

void MainWindow::onMultiConnect()
{
    if (netManager) { netManager->closeAll(); netManager->deleteLater(); netManager = nullptr; }
    localRematchRequested = false;
    remoteRematchRequested = false;
    netManager = new NetworkManager(this);
    connect(netManager, &NetworkManager::peerConnected,    this, &MainWindow::onNetPeerConnected);
    connect(netManager, &NetworkManager::peerDisconnected, this, &MainWindow::onNetPeerDisconnected);
    connect(netManager, &NetworkManager::messageReceived,  this, &MainWindow::onNetMessage);
    connect(netManager, &NetworkManager::networkError,     this, &MainWindow::onNetError);

    quint16 port = portEdit->text().toUShort();
    if (!port) port = 7777;
    iAmHost = hostRadio->isChecked();

    if (iAmHost) {
        if (netManager->host(port)) {
            netStatusLabel->setText(QString("🟡  대기 중... (포트:%1)\n상대방 접속 기다리는 중").arg(port));
            netStatusLabel->setStyleSheet("color:#ffcc00; border:1px solid #555; border-radius:6px; padding:10px;");
            connectBtn->setEnabled(false); cancelBtn->setEnabled(true);
            updateHeaderStatus("멀티 방 대기 중");
        }
    } else {
        QString ip = ipEdit->text().trimmed();
        if (ip.isEmpty()) ip = "127.0.0.1";
        netManager->join(ip, port);
        netStatusLabel->setText(QString("🟡  %1:%2 연결 중...").arg(ip).arg(port));
        netStatusLabel->setStyleSheet("color:#ffcc00; border:1px solid #555; border-radius:6px; padding:10px;");
        connectBtn->setEnabled(false); cancelBtn->setEnabled(true);
        updateHeaderStatus("멀티 연결 중");
    }
}

void MainWindow::onMultiCancel()
{
    if (netManager) { netManager->closeAll(); netManager->deleteLater(); netManager = nullptr; }
    localRematchRequested = false;
    remoteRematchRequested = false;
    netStatusLabel->setText("연결 취소됨");
    netStatusLabel->setStyleSheet("color:#888; border:1px solid #333; border-radius:6px; padding:8px;");
    connectBtn->setEnabled(true); cancelBtn->setEnabled(false);
    multiStartBtn->setEnabled(false);
    playerListLabel->setText("(없음)");
    ui->rematchBtn->setVisible(false);
    ui->leaveRoomBtn->setVisible(false);
    updateHeaderStatus("멀티 연결 취소됨");
}

void MainWindow::onNetPeerConnected()
{
    localRematchRequested = false;
    remoteRematchRequested = false;
    QString myName = multiPlayerCombo->currentText();
    QJsonObject hello; hello["t"] = "hello"; hello["name"] = myName;
    netManager->sendJson(hello);
    netStatusLabel->setText("🟢  연결 성공!");
    netStatusLabel->setStyleSheet("color:#00ff88; border:1px solid #555; border-radius:6px; padding:10px;");
    if (iAmHost) multiStartBtn->setEnabled(true);
    updateHeaderStatus("멀티 연결됨");
}

void MainWindow::onNetPeerDisconnected()
{
    if (netManager) {
        QObject::disconnect(netManager, nullptr, this, nullptr);
        netManager->closeAll();
        netManager->deleteLater();
        netManager = nullptr;
    }
    if (game) game->resetToIdle();
    localRematchRequested = false;
    remoteRematchRequested = false;
    remotePlayerName.clear();
    currentOpponentName = "AI";
    netStatusLabel->setText("🔴  상대방 연결 끊김");
    netStatusLabel->setStyleSheet("color:#ff5555; border:1px solid #555; border-radius:6px; padding:8px;");
    multiStartBtn->setEnabled(false);
    connectBtn->setEnabled(true); cancelBtn->setEnabled(false);
    playerListLabel->setText("(없음)");
    ui->pauseBtn->setEnabled(false);
    ui->rematchBtn->setVisible(false);
    ui->leaveRoomBtn->setVisible(false);
    statusLabel->setText("상대방 연결 끊김");
    updateHeaderStatus("멀티 연결 끊김");
}

void MainWindow::onNetError(QString err)
{
    netStatusLabel->setText("❌  오류: " + err);
    netStatusLabel->setStyleSheet("color:#ff5555; border:1px solid #555; border-radius:6px; padding:10px;");
    connectBtn->setEnabled(true); cancelBtn->setEnabled(false);
    updateHeaderStatus("멀티 오류");
}

void MainWindow::onNetMessage(QJsonObject obj)
{
    QString t = obj["t"].toString();
    if (t == "hello") {
        remotePlayerName = obj["name"].toString();
        playerListLabel->setText(
            QString("🔵  %1  (나)\n🟢  %2  (상대)")
            .arg(multiPlayerCombo->currentText(), remotePlayerName));
        if (!iAmHost)
            netStatusLabel->setText("🟢  연결됨 – 호스트가 시작할 때까지 대기 중...");
        updateHeaderStatus("멀티 연결됨 - " + remotePlayerName);
    }
    else if (t == "start") {
        int hostStone = obj["hostStone"].toInt(1);
        startNetworkGame(hostStone);
    }
    else if (t == "rematch") {
        remoteRematchRequested = true;
        if (!localRematchRequested) {
            ui->rematchBtn->setVisible(true);
            ui->rematchBtn->setEnabled(true);
            ui->rematchBtn->setText("↻  재도전");
            statusLabel->setText(remotePlayerName + "님이 재도전을 요청했습니다.");
            updateHeaderStatus("재도전 요청 받음");
        }
        tryStartRematch();
    }
    else if (t == "leave") {
        handleRoomLeft("상대방이 방을 나갔습니다.");
    }
    else {
        game->handleNetMessage(obj);
    }
}

void MainWindow::onMultiStart()
{
    int hostStone = QRandomGenerator::global()->bounded(2) + 1;
    QJsonObject msg;
    msg["t"] = "start";
    msg["hostStone"] = hostStone;
    netManager->sendJson(msg);
    startNetworkGame(hostStone);
}

void MainWindow::startNetworkGame(int hostStone)
{
    QString myName = multiPlayerCombo->currentText();
    if (remotePlayerName.isEmpty()) remotePlayerName = "상대방";
    currentOpponentName = remotePlayerName;
    localRematchRequested = false;
    remoteRematchRequested = false;
    const int myStone = iAmHost
        ? ((hostStone == 2) ? 2 : 1)
        : ((hostStone == 2) ? 1 : 2);
    game->startNetwork(myStone, netManager, myName, remotePlayerName);
    ui->pauseBtn->setEnabled(true);
    ui->pauseBtn->setText("⏸  일시정지");
    ui->rematchBtn->setVisible(false);
    ui->rematchBtn->setEnabled(false);
    ui->rematchBtn->setText("↻  재도전");
    ui->leaveRoomBtn->setVisible(true);
    ui->leaveRoomBtn->setEnabled(true);
    multiStartBtn->setEnabled(false);
    const QString stoneText = (myStone == 1) ? "흑" : "백";
    netStatusLabel->setText(QString("🟢  대전 시작! 내 돌: %1").arg(stoneText));
    updateHeaderStatus(QString("멀티 대전 중 - %1 (%2)").arg(remotePlayerName, stoneText));
    tabs->setCurrentIndex(0);
    QTimer::singleShot(100, this, [this]{ game->setFocus(); });
}

void MainWindow::tryStartRematch()
{
    if (!localRematchRequested || !remoteRematchRequested || !iAmHost
        || !netManager || !netManager->isConnected()) {
        return;
    }

    const int hostStone = QRandomGenerator::global()->bounded(2) + 1;
    QJsonObject msg;
    msg["t"] = "start";
    msg["hostStone"] = hostStone;
    netManager->sendJson(msg);
    startNetworkGame(hostStone);
}

void MainWindow::handleRoomLeft(const QString& message)
{
    if (netManager) {
        QObject::disconnect(netManager, nullptr, this, nullptr);
        netManager->closeAll();
        netManager->deleteLater();
        netManager = nullptr;
    }

    if (game) game->resetToIdle();
    localRematchRequested = false;
    remoteRematchRequested = false;
    remotePlayerName.clear();
    currentOpponentName = "AI";

    ui->pauseBtn->setEnabled(false);
    ui->pauseBtn->setText("⏸  일시정지");
    ui->rematchBtn->setVisible(false);
    ui->leaveRoomBtn->setVisible(false);
    multiStartBtn->setEnabled(false);
    connectBtn->setEnabled(true);
    cancelBtn->setEnabled(false);
    playerListLabel->setText("(없음)");
    netStatusLabel->setText(message);
    netStatusLabel->setStyleSheet("color:#888; border:1px solid #333; border-radius:6px; padding:8px;");
    statusLabel->setText(message);
    updateHeaderStatus(message);
}

QString MainWindow::currentPlayerName() const
{
    return loggedInPlayer_.nickname;
}

void MainWindow::applyLoggedInPlayer(const Player& player)
{
    loggedInPlayer_ = player;
    selectedPlayerId = player.id;
    remotePlayerName.clear();
    currentOpponentName = "AI";
    if (game) game->resetToIdle();
    ui->pauseBtn->setEnabled(false);
    ui->pauseBtn->setText("⏸  일시정지");
    ui->rematchBtn->setVisible(false);
    ui->leaveRoomBtn->setVisible(false);

    setWindowTitle(QString("⚫⚪  오목 게임 - %1").arg(loggedInPlayer_.nickname));
    ui->accountLabel->setText("로그인: " + loggedInPlayer_.nickname);
    refreshPlayerList();
    refreshMyRecordsTable();
    refreshRecordTable();
    selectedOpponent_.clear();
    if (opponentNameLabel) opponentNameLabel->setText("(선택 안됨)");
    refreshOpponentStats(QString());
    statusLabel->setText("플레이어를 선택하고 게임을 시작하세요.");
    updateHeaderStatus("게임 대기 중");
}

void MainWindow::updateHeaderStatus(const QString& text)
{
    if (!text.isEmpty()) {
        ui->headerStatusLabel->setText(text);
        return;
    }

    switch (tabs->currentIndex()) {
    case 0:
        ui->headerStatusLabel->setText(game && game->isPlaying() ? "게임 진행 중" : "게임 대기 중");
        break;
    case 1:
        ui->headerStatusLabel->setText("내 프로필");
        break;
    case 2:
        ui->headerStatusLabel->setText("전적 확인");
        break;
    case 3:
        ui->headerStatusLabel->setText("플레이어 조회");
        break;
    case 4:
        ui->headerStatusLabel->setText(netManager ? "멀티 연결 관리" : "멀티 대기");
        break;
    default:
        ui->headerStatusLabel->setText("오목 게임");
        break;
    }
}
