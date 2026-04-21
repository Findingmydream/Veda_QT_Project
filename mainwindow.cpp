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
#include "recordstable.h"
#include "titles.h"

namespace {
QString stoneName(int stone)
{
    return stone == 1 ? QStringLiteral("흑") : QStringLiteral("백");
}
}

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
    setupProfileTab();
    setupRecordTab();
    setupMyRecordsTab();
    setupMultiTab();

    // 멀티 탭을 게임 탭 바로 오른쪽(인덱스 1) 으로 이동.
    // 최종 순서: 게임(0) / 멀티(1) / 내 프로필(2) / 플레이어 조회(3) / 전적(4)
    {
        int multiIdx = tabs->indexOf(ui->multiTab);
        if (multiIdx > 1) tabs->tabBar()->moveTab(multiIdx, 1);
    }

    setWindowTitle(QString("⚫⚪  오목 게임 - %1").arg(loggedInPlayer_.nickname));
    ui->accountLabel->setText("로그인: " + loggedInPlayer_.nickname);

    connect(tabs, &QTabWidget::currentChanged, [this](int i) {
        // 새 순서: 0=게임 / 1=멀티 / 2=내 프로필 / 3=플레이어 조회 / 4=전적
        if (i == 1)   refreshPlayerList();                                 // 멀티
        if (i == 2)   refreshPlayerList();                                 // 내 프로필
        if (i == 3) { refreshPlayerList(); refreshRecordTable(); }         // 플레이어 조회
        if (i == 4) { refreshPlayerList(); refreshMyRecordsTable(); }      // 전적
        updateHeaderStatus();
    });

    refreshPlayerList();
    updateHeaderStatus();
    tabs->setCurrentIndex(0);   // 로그인 후 기본은 게임 탭
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

    // AI 난이도 선택 버튼 (↓ 모양 메뉴로 열리는 드롭다운).
    // 게임 설정 레이아웃의 AI 대전 시작 버튼 왼쪽에 삽입.
    aiDifficultyBtn = new QPushButton(this);
    aiDifficultyBtn->setMinimumSize(130, 32);
    auto* diffMenu = new QMenu(aiDifficultyBtn);
    auto updateDiffLabel = [this]() {
        const char* name = "쉬움";
        if      (currentAIDifficulty == GameWidget::AIDifficulty::Medium) name = "중간";
        else if (currentAIDifficulty == GameWidget::AIDifficulty::Hard)   name = "어려움(Gemini)";
        aiDifficultyBtn->setText(QString("🎚  난이도: %1 ▾").arg(name));
    };
    QAction* easyAct   = diffMenu->addAction("쉬움");
    QAction* mediumAct = diffMenu->addAction("중간");
    QAction* hardAct   = diffMenu->addAction("어려움 (Gemini)");
    connect(easyAct, &QAction::triggered, this, [this, updateDiffLabel]() {
        currentAIDifficulty = GameWidget::AIDifficulty::Easy;
        updateDiffLabel();
    });
    connect(mediumAct, &QAction::triggered, this, [this, updateDiffLabel]() {
        currentAIDifficulty = GameWidget::AIDifficulty::Medium;
        updateDiffLabel();
    });
    connect(hardAct, &QAction::triggered, this, [this, updateDiffLabel]() {
        currentAIDifficulty = GameWidget::AIDifficulty::Hard;
        updateDiffLabel();
    });
    aiDifficultyBtn->setMenu(diffMenu);
    updateDiffLabel();

    if (auto* startLayout = qobject_cast<QHBoxLayout*>(ui->startSingleBtn->parentWidget()->layout())) {
        int startIdx = startLayout->indexOf(ui->startSingleBtn);
        startLayout->insertWidget(startIdx, aiDifficultyBtn);
    }

    // turnTimerLabel(30초) 아래에 목숨 표시를 넣기 위해 컨테이너 위젯으로 감쌈.
    livesLabel = new QLabel(this);
    livesLabel->setAlignment(Qt::AlignCenter);
    livesLabel->setStyleSheet("color:#ff6b6b; font-size:11px; padding:0 4px;");
    livesLabel->setText("❤ ❤ ❤");
    if (auto* settingLayout = qobject_cast<QHBoxLayout*>(ui->turnTimerLabel->parentWidget()->layout())) {
        int timerIdx = settingLayout->indexOf(ui->turnTimerLabel);
        if (timerIdx >= 0) {
            settingLayout->takeAt(timerIdx);  // HBox 에서 라벨 떼어냄 (삭제는 안 됨)

            auto* timerCol = new QVBoxLayout;
            timerCol->setSpacing(1);
            timerCol->setContentsMargins(0, 0, 0, 0);
            timerCol->addWidget(ui->turnTimerLabel);
            timerCol->addWidget(livesLabel);
            settingLayout->insertLayout(timerIdx, timerCol);
        }
    }

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
    connect(game, &GameWidget::livesChanged, [this](int livesBlack, int livesWhite) {
        if (!livesLabel) return;
        // 내 돌 색에 맞춰 내 목숨만 표시. 플레이어 기준이라 "내 목숨 = ❤ x N" 느낌.
        int myStone = game->myStoneColor();
        int lives = (myStone == 2) ? livesWhite : livesBlack;  // 기본 흑 기준
        const int max = 3;
        QString txt;
        for (int i = 0; i < max; ++i) txt += (i < lives) ? "❤ " : "🖤 ";
        livesLabel->setText(txt.trimmed());
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
    game->startSingle(name, first, currentAIDifficulty);
    ui->pauseBtn->setEnabled(true);
    ui->pauseBtn->setText("⏸  일시정지");
    ui->rematchBtn->setVisible(false);
    ui->leaveRoomBtn->setVisible(false);
    const char* diffName = "쉬움";
    if      (currentAIDifficulty == GameWidget::AIDifficulty::Medium) diffName = "중간";
    else if (currentAIDifficulty == GameWidget::AIDifficulty::Hard)   diffName = "어려움";
    updateHeaderStatus(QString("AI 대전 중 (%1) - %2 선공").arg(diffName, first ? name : "AI"));
    statusLabel->setText(QString("%1 vs AI (%2)  |  %3 선공")
        .arg(name, diffName, first ? name : "AI"));
}

void MainWindow::onGameFinished(QString winner, int moves, int durationSeconds, int myStone)
{
    QString name = currentPlayerName();
    Player p = Database::instance().readPlayerByName(name);
    if (p.id <= 0) return;

    QString result;
    if (winner.isEmpty())   result = "무";
    else if (winner == name) result = "승";
    else                     result = "패";

    QString opponent = currentOpponentName.isEmpty() ? "AI" : currentOpponentName;
    Database::instance().createRecord(p.id, opponent, result, moves, durationSeconds, myStone);
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
    remoteRematchRequested = false;
    ui->rematchBtn->setEnabled(false);
    ui->rematchBtn->setText("상대 대기 중");
    statusLabel->setText("재도전 요청을 보냈습니다.");
    updateHeaderStatus("재도전 대기 중");

    QJsonObject msg;
    msg["t"] = "rematch_request";
    msg["name"] = multiPlayerCombo->currentText();
    netManager->sendJson(msg);
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
    headerRow->addLayout(avatarCol, 0);

    auto* avatarBtnRow = new QHBoxLayout;
    avatarBtnRow->setSpacing(4);
    avatarEditBtn    = new QPushButton("📷  사진 변경");
    avatarCaptureBtn = new QPushButton("📸  사진 생성");
    accountDeleteBtn = new QPushButton("계정 삭제");
    avatarBtnRow->addWidget(avatarEditBtn);
    avatarBtnRow->addWidget(avatarCaptureBtn);
    avatarBtnRow->addStretch(1);
    avatarBtnRow->addWidget(accountDeleteBtn);

    // 우: 닉네임 + 칭호
    auto* infoCol = new QVBoxLayout;
    infoCol->setSpacing(8);

    auto* nickRow = new QHBoxLayout;
    profileNickLabel = new QLabel;
    profileNickLabel->setStyleSheet("color:white; font-size:22px; font-weight:bold;");
    profileNickEdit = new QLineEdit;
    profileNickEdit->setStyleSheet("color:white; font-size:18px; font-weight:bold; padding:2px 6px;");
    profileNickEdit->hide();
    nickEditBtn = new QPushButton("수정");
    nickEditBtn->setStyleSheet("font-size:13px; padding:4px 14px;");
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
    root->addLayout(avatarBtnRow);

    // === 전적 sub-tabs ===
    auto* statsTabs = new QTabWidget;

    // --- Tab 1: 내 전적 (기록 테이블만) ---
    auto* myTab = new QWidget;
    auto* myLay = new QVBoxLayout(myTab);
    myRecentTable = new QTableWidget;
    RecordsTable::setupColumns(myRecentTable, /*includePlayerColumn=*/false);
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
    RecordsTable::setupColumns(opponentRecordTable, /*includePlayerColumn=*/false);
    oppLay->addWidget(opponentRecordTable, 1);

    statsTabs->addTab(oppTab, "상대 전적");

    root->addWidget(statsTabs, 1);

    tabs->insertTab(1, w, "👤  내 프로필");

    connect(avatarEditBtn,     &QPushButton::clicked, this, &MainWindow::onAvatarEditClicked);
    connect(avatarCaptureBtn,  &QPushButton::clicked, this, &MainWindow::onAvatarCaptureClicked);
    connect(accountDeleteBtn,  &QPushButton::clicked, this, &MainWindow::onDeleteAccount);
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

void MainWindow::onDeleteAccount()
{
    if (loggedInPlayer_.id <= 0) {
        QMessageBox::warning(this, "알림", "로그인된 계정 정보가 없습니다.");
        return;
    }

    if (accountDeleteBtn) accountDeleteBtn->setEnabled(false);

    if (QMessageBox::warning(this, "계정 삭제", "정말로 삭제하시겠습니까?",
                             QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::No) != QMessageBox::Yes) {
        if (accountDeleteBtn) accountDeleteBtn->setEnabled(true);
        return;
    }

    if (netManager) {
        netManager->closeAll();
        netManager->deleteLater();
        netManager = nullptr;
    }

    const int playerId = loggedInPlayer_.id;
    if (!Database::instance().deletePlayer(playerId)) {
        QMessageBox::warning(this, "알림", "계정 삭제에 실패했습니다.");
        if (accountDeleteBtn) accountDeleteBtn->setEnabled(true);
        return;
    }

    loggedInPlayer_ = Player();
    selectedOpponent_.clear();

    hide();
    LoginDialog login;
    if (login.exec() == QDialog::Accepted) {
        applyLoggedInPlayer(login.loggedInPlayer());
        if (accountDeleteBtn) accountDeleteBtn->setEnabled(true);
        show();
        tabs->setCurrentIndex(0);
    } else {
        close();
    }
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
            if (!Database::instance().updatePlayer(loggedInPlayer_.id, newNick, loggedInPlayer_.comment)) {
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
        nickEditBtn->setText("수정");
        refreshPlayerList();
    } else {
        // 편집 모드 진입
        profileNickEdit->setText(loggedInPlayer_.nickname);
        profileNickLabel->hide();
        profileNickEdit->show();
        profileNickEdit->setFocus();
        profileNickEdit->selectAll();
        nickEditBtn->setText("저장");
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
        RecordsTable::fill(myRecentTable, records, /*includePlayerColumn=*/false);
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

    RecordsTable::fill(opponentRecordTable, matched, /*includePlayerColumn=*/false);
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
    recordPlayerListTable->setHorizontalHeaderLabels({"", "닉네임", "칭호", "승률", "총판"});
    recordPlayerListTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    recordPlayerListTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    recordPlayerListTable->setAlternatingRowColors(true);
    recordPlayerListTable->verticalHeader()->setVisible(false);
    root->addWidget(recordPlayerListTable, 1);

    tabs->insertTab(2, w, "🔍  플레이어 조회");

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
    myRecordsDeleteAllBtn = new QPushButton("🗑  전체 삭제");
    topRow->addWidget(new QLabel("결과:"));
    topRow->addWidget(myRecordsResultCombo);
    topRow->addStretch(1);
    topRow->addWidget(refreshBtn);
    topRow->addWidget(myRecordsDeleteAllBtn);
    root->addLayout(topRow);

    myRecordsTable = new QTableWidget;
    RecordsTable::setupColumns(myRecordsTable, /*includePlayerColumn=*/true);
    root->addWidget(myRecordsTable, 1);

    tabs->insertTab(3, w, "📊  전적");

    connect(refreshBtn,           &QPushButton::clicked, this, &MainWindow::onMyRecordsRefresh);
    connect(myRecordsResultCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int) { refreshMyRecordsTable(); });
    connect(myRecordsDeleteAllBtn, &QPushButton::clicked, this, &MainWindow::onDeleteAllMyRecords);
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
        auto* btn = new QPushButton("📋  상세보기");
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
        setC(3, QString::number(p.winRate(), 'f', 1) + "%");
        setC(4, QString::number(p.totalGames()));
    }

    auto* hh = recordPlayerListTable->horizontalHeader();
    hh->setStretchLastSection(false);
    hh->setSectionResizeMode(0, QHeaderView::Fixed);
    hh->setSectionResizeMode(1, QHeaderView::Fixed);
    hh->setSectionResizeMode(2, QHeaderView::Stretch);
    hh->setSectionResizeMode(3, QHeaderView::Stretch);
    hh->setSectionResizeMode(4, QHeaderView::Stretch);
    recordPlayerListTable->setColumnWidth(0, 130);
    recordPlayerListTable->setColumnWidth(1, 120);
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

    QString resultFilter = myRecordsResultCombo ? myRecordsResultCombo->currentText() : QString();
    if (resultFilter == "전체") resultFilter.clear();

    // 모든 유저의 기록을 시간순으로 (readRecords 가 played_at DESC 로 정렬)
    auto records = Database::instance().readRecords(0, resultFilter);
    RecordsTable::fill(myRecordsTable, records, /*includePlayerColumn=*/true);
}

void MainWindow::onDeleteAllMyRecords()
{
    if (QMessageBox::question(this, "확인", "정말로 삭제하겠습니까?")
        != QMessageBox::Yes) return;

    if (!Database::instance().deleteAllRecords(0)) {
        QMessageBox::warning(this, "알림", "전체 기록 삭제에 실패했습니다.");
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
    handlePeerLeft("🔴  상대방 연결 끊김");
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
        QString blackName = obj["blackName"].toString();
        QString whiteName = obj["whiteName"].toString();
        startNetworkGameWithStone(iAmHost
            ? ((hostStone == 2) ? 2 : 1)
            : ((hostStone == 2) ? 1 : 2),
            blackName, whiteName);
    }
    else if (t == "rematch_request" || t == "rematch") {
        remoteRematchRequested = true;
        QString requester = obj["name"].toString();
        if (requester.isEmpty()) requester = remotePlayerName.isEmpty() ? "상대방" : remotePlayerName;

        if (localRematchRequested) {
            QJsonObject accepted;
            accepted["t"] = "rematch_accept";
            netManager->sendJson(accepted);
            if (iAmHost) tryStartRematch();
            return;
        }

        QMessageBox box(this);
        box.setWindowTitle("재도전 요청");
        box.setText(requester + "님이 재도전을 요청했습니다.");
        box.setInformativeText("한 판 더 진행할까요?");
        auto* acceptBtn = box.addButton("수락", QMessageBox::AcceptRole);
        box.addButton("거절", QMessageBox::RejectRole);
        box.setDefaultButton(qobject_cast<QPushButton*>(acceptBtn));
        box.exec();

        if (box.clickedButton() == acceptBtn) {
            localRematchRequested = true;
            statusLabel->setText("재도전을 수락했습니다.");
            updateHeaderStatus("재도전 시작 준비");
            QJsonObject accepted;
            accepted["t"] = "rematch_accept";
            netManager->sendJson(accepted);
            if (iAmHost) tryStartRematch();
        } else {
            QJsonObject rejected;
            rejected["t"] = "rematch_reject";
            netManager->sendJson(rejected);
            handleRoomLeft("재도전 요청을 거절했습니다.");
        }
    }
    else if (t == "rematch_accept") {
        remoteRematchRequested = true;
        statusLabel->setText("상대방이 재도전을 수락했습니다.");
        updateHeaderStatus("재도전 시작 준비");
        if (iAmHost) tryStartRematch();
    }
    else if (t == "rematch_reject") {
        handleRoomLeft("상대방이 재도전을 거절했습니다.");
    }
    else if (t == "leave") {
        handlePeerLeft("상대방이 방을 나갔습니다.");
    }
    else {
        game->handleNetMessage(obj);
    }
}

void MainWindow::onMultiStart()
{
    QString hostName = multiPlayerCombo->currentText();
    QString guestName = remotePlayerName.isEmpty() ? "상대방" : remotePlayerName;
    const bool hostBlack = (QRandomGenerator::global()->bounded(2) == 0);
    const int hostStone = hostBlack ? 1 : 2;
    const QString blackName = hostBlack ? hostName : guestName;
    const QString whiteName = hostBlack ? guestName : hostName;

    QJsonObject msg;
    msg["t"] = "start";
    msg["hostStone"] = hostStone;
    msg["blackName"] = blackName;
    msg["whiteName"] = whiteName;
    netManager->sendJson(msg);
    startNetworkGameWithStone(hostStone, blackName, whiteName);
}

void MainWindow::startNetworkGame(int hostStone)
{
    const int myStone = iAmHost
        ? ((hostStone == 2) ? 2 : 1)
        : ((hostStone == 2) ? 1 : 2);
    startNetworkGameWithStone(myStone);
}

void MainWindow::startNetworkGameWithStone(int myStone, const QString& blackName,
                                           const QString& whiteName)
{
    QString myName = multiPlayerCombo->currentText();
    if (remotePlayerName.isEmpty()) remotePlayerName = "상대방";
    currentOpponentName = remotePlayerName;
    localRematchRequested = false;
    remoteRematchRequested = false;

    game->startNetwork(myStone, netManager, myName, remotePlayerName);
    ui->pauseBtn->setEnabled(true);
    ui->pauseBtn->setText("⏸  일시정지");
    ui->rematchBtn->setVisible(false);
    ui->rematchBtn->setEnabled(false);
    ui->rematchBtn->setText("↻  재도전");
    ui->leaveRoomBtn->setVisible(true);
    ui->leaveRoomBtn->setEnabled(true);
    multiStartBtn->setEnabled(false);

    // 멀티에서 배정된 색을 선/후공 라디오에 반영한다. 선택은 불가(표시 전용).
    const bool iAmBlack = (myStone == 1);
    ui->firstRadio->setChecked(iAmBlack);
    ui->secondRadio->setChecked(!iAmBlack);
    ui->firstRadio->setEnabled(false);
    ui->secondRadio->setEnabled(false);

    const int opponentStone = (myStone == 1) ? 2 : 1;
    QString black = blackName;
    QString white = whiteName;
    if (black.isEmpty() || white.isEmpty()) {
        black = (myStone == 1) ? myName : remotePlayerName;
        white = (myStone == 2) ? myName : remotePlayerName;
    }

    playerListLabel->setText(
        QString("🔵  %1  (나, %2)\n🟢  %3  (상대, %4)")
            .arg(myName, stoneName(myStone), remotePlayerName, stoneName(opponentStone)));
    netStatusLabel->setText(QString("🟢  대전 시작! 흑: %1 / 백: %2")
        .arg(black, white));
    statusLabel->setText(QString("%1(%2) vs %3(%4)")
        .arg(myName, stoneName(myStone), remotePlayerName, stoneName(opponentStone)));
    updateHeaderStatus(QString("멀티 대전 중 - 내 돌: %1").arg(stoneName(myStone)));
    tabs->setCurrentIndex(0);
    QTimer::singleShot(100, this, [this]{ game->setFocus(); });
}

void MainWindow::tryStartRematch()
{
    if (!localRematchRequested || !remoteRematchRequested || !iAmHost
        || !netManager || !netManager->isConnected()) {
        return;
    }

    QString hostName = multiPlayerCombo->currentText();
    QString guestName = remotePlayerName.isEmpty() ? "상대방" : remotePlayerName;
    const bool hostBlack = (QRandomGenerator::global()->bounded(2) == 0);
    const int hostStone = hostBlack ? 1 : 2;
    const QString blackName = hostBlack ? hostName : guestName;
    const QString whiteName = hostBlack ? guestName : hostName;

    QJsonObject msg;
    msg["t"] = "start";
    msg["hostStone"] = hostStone;
    msg["blackName"] = blackName;
    msg["whiteName"] = whiteName;
    netManager->sendJson(msg);
    startNetworkGameWithStone(hostStone, blackName, whiteName);
}

void MainWindow::handlePeerLeft(const QString& message)
{
    if (iAmHost && netManager && netManager->isHosting()) {
        netManager->closePeer();

        if (game) game->resetToIdle();
        localRematchRequested = false;
        remoteRematchRequested = false;
        remotePlayerName.clear();
        currentOpponentName = "AI";

        ui->pauseBtn->setEnabled(false);
        ui->pauseBtn->setText("⏸  일시정지");
        ui->rematchBtn->setVisible(false);
        ui->leaveRoomBtn->setVisible(false);
        ui->firstRadio->setEnabled(true);
        ui->secondRadio->setEnabled(true);
        multiStartBtn->setEnabled(false);
        connectBtn->setEnabled(false);
        cancelBtn->setEnabled(true);
        playerListLabel->setText(
            QString("🔵  %1  (나)\n(상대 대기 중)").arg(multiPlayerCombo->currentText()));
        netStatusLabel->setText(message + "\n상대방 접속 기다리는 중");
        netStatusLabel->setStyleSheet("color:#ffcc00; border:1px solid #555; border-radius:6px; padding:10px;");
        statusLabel->setText(message);
        updateHeaderStatus("멀티 방 대기 중");

        int multiIdx = tabs->indexOf(ui->multiTab);
        if (multiIdx >= 0) tabs->setCurrentIndex(multiIdx);
        return;
    }

    handleRoomLeft(message);
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
    // 멀티 전용으로 잠가뒀던 선/후공 라디오를 다시 싱글 AI 대전용으로 풀어준다.
    ui->firstRadio->setEnabled(true);
    ui->secondRadio->setEnabled(true);
    multiStartBtn->setEnabled(false);
    connectBtn->setEnabled(true);
    cancelBtn->setEnabled(false);
    playerListLabel->setText("(없음)");
    netStatusLabel->setText(message);
    netStatusLabel->setStyleSheet("color:#888; border:1px solid #333; border-radius:6px; padding:8px;");
    statusLabel->setText(message);
    updateHeaderStatus(message);

    // 방을 나가면 게임 탭에 머무르지 않고 멀티 로비로 돌려보낸다.
    int multiIdx = tabs->indexOf(ui->multiTab);
    if (multiIdx >= 0) tabs->setCurrentIndex(multiIdx);
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
    ui->firstRadio->setEnabled(true);
    ui->secondRadio->setEnabled(true);

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
        ui->headerStatusLabel->setText(netManager ? "멀티 연결 관리" : "멀티 대기");
        break;
    case 2:
        ui->headerStatusLabel->setText("내 프로필");
        break;
    case 3:
        ui->headerStatusLabel->setText("플레이어 조회");
        break;
    case 4:
        ui->headerStatusLabel->setText("전적 확인");
        break;
    default:
        ui->headerStatusLabel->setText("오목 게임");
        break;
    }
}
