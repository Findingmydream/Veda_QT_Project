#include "mainwindow.h"
#include "database.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFont>
#include <QJsonObject>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QTimer>

// ─────────────────────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , tabs(new QTabWidget(this))
{
    Database::instance().init();

    setupGameTab();
    setupRecordsTab();
    setupMultiTab();

    setCentralWidget(tabs);
    setWindowTitle("🚀  우주 슈팅 게임");
    setMinimumSize(460, 720);

    setStyleSheet(R"(
        QMainWindow { background: #1a1a2e; }
        QTabWidget::pane { border: 1px solid #2a2a4a; background: #16213e; }
        QTabBar::tab { background: #0f3460; color: #ccc; padding: 8px 18px;
                       font-size: 12px; font-weight: bold; }
        QTabBar::tab:selected { background: #e94560; color: white; }
        QTabBar::tab:hover    { background: #1a4a80; }
        QGroupBox { color: #8888aa; border: 1px solid #2a2a4a; border-radius: 6px;
                    margin-top: 10px; padding-top: 6px; font-weight: bold; }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; top: -2px; }
        QPushButton { background: #0f3460; color: white; border: none;
                      padding: 7px 18px; border-radius: 5px; font-weight: bold; }
        QPushButton:hover   { background: #e94560; }
        QPushButton:pressed { background: #b03040; }
        QPushButton:disabled { background: #222244; color: #555; }
        QLineEdit, QDateEdit { background: #0d1117; color: white;
                               border: 1px solid #2a2a4a; border-radius: 4px; padding: 5px 8px; }
        QComboBox { background: #0d1117; color: white;
                    border: 1px solid #2a2a4a; border-radius: 4px; padding: 5px 8px; }
        QComboBox::drop-down { border: none; width: 18px; }
        QRadioButton { color: #ddd; }
        QTableWidget { background: #0d1117; color: white; gridline-color: #1e1e3a;
                       border: 1px solid #2a2a4a; selection-background-color: #0f3460; }
        QTableWidget QHeaderView::section { background: #0f3460; color: #ddd;
                                            padding: 5px; border: none; font-weight: bold; }
        QTableWidget::item:alternate { background: #0e1525; }
        QLabel { color: white; }
        QScrollBar:vertical { background: #0d1117; width: 8px; border-radius: 4px; }
        QScrollBar::handle:vertical { background: #333; border-radius: 4px; }
    )");

    connect(tabs, &QTabWidget::currentChanged, [this](int idx) {
        if (idx == 1) refreshRecords();
    });
}

// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupGameTab()
{
    auto* w      = new QWidget;
    auto* layout = new QVBoxLayout(w);
    layout->setSpacing(8);
    layout->setContentsMargins(10, 10, 10, 6);

    auto* nameGroup  = new QGroupBox("플레이어");
    auto* nameLayout = new QHBoxLayout(nameGroup);
    nameEdit = new QLineEdit("Player1");
    nameEdit->setMaxLength(20);
    difficultyCombo = new QComboBox;
    difficultyCombo->addItems({"쉬움", "보통", "어려움"});
    difficultyCombo->setCurrentIndex(0);
    difficultyCombo->setFixedWidth(86);
    startBtn = new QPushButton("▶  시작");
    startBtn->setFixedWidth(110);
    nameLayout->addWidget(nameEdit);
    nameLayout->addWidget(new QLabel("난이도"));
    nameLayout->addWidget(difficultyCombo);
    nameLayout->addWidget(startBtn);
    layout->addWidget(nameGroup);

    auto* statsRow = new QHBoxLayout;
    scoreLabel = new QLabel("점수: 0");
    livesLabel = new QLabel("♥ ♥ ♥");
    levelLabel = new QLabel("레벨: 1");
    QFont sf; sf.setBold(true); sf.setPointSize(12);
    for (auto* l : {scoreLabel, livesLabel, levelLabel}) {
        l->setFont(sf); l->setAlignment(Qt::AlignCenter); statsRow->addWidget(l);
    }
    livesLabel->setStyleSheet("color: #ff5555;");
    levelLabel->setStyleSheet("color: #00ddff;");
    layout->addLayout(statsRow);

    game = new GameWidget;
    layout->addWidget(game, 0, Qt::AlignHCenter);

    auto* hint = new QLabel("← → (또는 A D) 이동    자동 발사    게임창 클릭 후 키 입력");
    hint->setAlignment(Qt::AlignCenter);
    hint->setStyleSheet("color: #555577; font-size: 10px;");
    layout->addWidget(hint);

    tabs->addTab(w, "🚀  게임");

    connect(startBtn, &QPushButton::clicked,    this, &MainWindow::onStartPause);
    connect(game,     &GameWidget::gameOver,     this, &MainWindow::onGameOver);
    connect(game,     &GameWidget::statsChanged, this, &MainWindow::onStatsChanged);
}

// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupRecordsTab()
{
    auto* w = new QWidget;
    auto* layout = new QVBoxLayout(w);
    layout->setSpacing(8);
    layout->setContentsMargins(10, 10, 10, 10);

    auto* sg = new QGroupBox("🔍  검색 필터");
    auto* sl = new QVBoxLayout(sg);

    auto* r1 = new QHBoxLayout;
    r1->addWidget(new QLabel("이름 :"));
    searchName = new QLineEdit;
    searchName->setPlaceholderText("이름으로 검색...");
    r1->addWidget(searchName);
    sl->addLayout(r1);

    auto* r2 = new QHBoxLayout;
    r2->addWidget(new QLabel("기간 :"));
    fromDate = new QDateEdit(QDate::currentDate().addMonths(-1));
    fromDate->setCalendarPopup(true); fromDate->setDisplayFormat("yyyy-MM-dd");
    toDate   = new QDateEdit(QDate::currentDate());
    toDate->setCalendarPopup(true);   toDate->setDisplayFormat("yyyy-MM-dd");
    r2->addWidget(fromDate); r2->addWidget(new QLabel("~")); r2->addWidget(toDate);
    sl->addLayout(r2);

    auto* r3 = new QHBoxLayout;
    auto* searchBtn = new QPushButton("🔍  검색");
    auto* resetBtn  = new QPushButton("↺  전체 보기");
    r3->addWidget(searchBtn); r3->addWidget(resetBtn);
    sl->addLayout(r3);
    layout->addWidget(sg);

    summaryLabel = new QLabel("총 0개의 기록");
    summaryLabel->setStyleSheet("color: #6666aa; font-size: 10px;");
    layout->addWidget(summaryLabel);

    recordTable = new QTableWidget;
    recordTable->setColumnCount(7);
    recordTable->setHorizontalHeaderLabels(
        {"순위", "이름", "점수", "처치 수", "레벨", "플레이 시간", "날짜"});
    recordTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    recordTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    recordTable->setAlternatingRowColors(true);
    recordTable->verticalHeader()->setVisible(false);
    recordTable->horizontalHeader()->setStretchLastSection(true);
    recordTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    layout->addWidget(recordTable, 1);

    tabs->addTab(w, "📊  전적");

    connect(searchBtn,  &QPushButton::clicked, this, &MainWindow::onSearchClicked);
    connect(resetBtn,   &QPushButton::clicked, this, &MainWindow::onResetClicked);
    connect(searchName, &QLineEdit::returnPressed, this, &MainWindow::onSearchClicked);

    refreshRecords();
}

// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupMultiTab()
{
    auto* w = new QWidget;
    auto* layout = new QVBoxLayout(w);
    layout->setSpacing(10);
    layout->setContentsMargins(14, 14, 14, 14);

    // ── Mode ─────────────────────────────────────────────────────────────────
    auto* modeGroup  = new QGroupBox("모드 선택");
    auto* modeLayout = new QVBoxLayout(modeGroup);
    hostRadio = new QRadioButton("🏠  방 만들기 (Host) – 이 PC가 서버");
    joinRadio = new QRadioButton("🔌  방 참가 (Join)  – 호스트에 접속");
    hostRadio->setChecked(true);
    modeLayout->addWidget(hostRadio);
    modeLayout->addWidget(joinRadio);
    layout->addWidget(modeGroup);

    // ── Settings ─────────────────────────────────────────────────────────────
    auto* settingsGroup  = new QGroupBox("접속 설정");
    auto* settingsLayout = new QFormLayout(settingsGroup);
    settingsLayout->setLabelAlignment(Qt::AlignRight);

    // 내 IP 자동 표시 (읽기 전용)
    auto* myIpEdit = new QLineEdit;
    myIpEdit->setReadOnly(true);
    myIpEdit->setStyleSheet("background: #0a0a1a; color: #00ff88; border: 1px solid #2a2a4a; border-radius:4px; padding:5px 8px;");
    QString localIp = "알 수 없음";
    for (const QNetworkInterface& iface : QNetworkInterface::allInterfaces()) {
        if (iface.flags().testFlag(QNetworkInterface::IsLoopBack)) continue;
        if (!iface.flags().testFlag(QNetworkInterface::IsUp))      continue;
        for (const QNetworkAddressEntry& entry : iface.addressEntries()) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                localIp = entry.ip().toString();
                break;
            }
        }
        if (localIp != "알 수 없음") break;
    }
    myIpEdit->setText(localIp);

    multiNameEdit = new QLineEdit("Player2");
    multiNameEdit->setMaxLength(20);
    portEdit = new QLineEdit("7777");
    ipEdit   = new QLineEdit("127.0.0.1");
    ipEdit->setEnabled(false);
    ipEdit->setPlaceholderText("호스트 IP 입력 (예: 192.168.0.10)");

    settingsLayout->addRow("내 IP :",     myIpEdit);
    settingsLayout->addRow("내 이름 :",   multiNameEdit);
    settingsLayout->addRow("포트 :",      portEdit);
    settingsLayout->addRow("호스트 IP :", ipEdit);
    layout->addWidget(settingsGroup);

    // ── Connect / Cancel buttons ──────────────────────────────────────────────
    auto* btnRow = new QHBoxLayout;
    connectBtn   = new QPushButton("🌐  방 만들기");
    cancelBtn    = new QPushButton("✕  취소");
    connectBtn->setFixedHeight(38);
    cancelBtn->setFixedHeight(38);
    cancelBtn->setEnabled(false);
    cancelBtn->setStyleSheet(
        "QPushButton:enabled  { background: #663333; color: white; }"
        "QPushButton:disabled { background: #222233; color: #555;  }");
    btnRow->addWidget(connectBtn);
    btnRow->addWidget(cancelBtn);
    layout->addLayout(btnRow);

    // ── Status ────────────────────────────────────────────────────────────────
    netStatusLabel = new QLabel("연결되지 않음");
    netStatusLabel->setAlignment(Qt::AlignCenter);
    netStatusLabel->setStyleSheet("color: #888; font-size: 12px; "
                                   "border: 1px solid #333; border-radius: 6px; padding: 8px;");
    layout->addWidget(netStatusLabel);

    // ── Player list ───────────────────────────────────────────────────────────
    auto* playerGroup  = new QGroupBox("참가자");
    auto* playerLayout = new QVBoxLayout(playerGroup);
    playerListLabel = new QLabel("(없음)");
    playerListLabel->setAlignment(Qt::AlignCenter);
    playerListLabel->setStyleSheet("color: #aaa; font-size: 12px; padding: 6px;");
    playerLayout->addWidget(playerListLabel);
    layout->addWidget(playerGroup);

    // ── Start game ────────────────────────────────────────────────────────────
    multiStartBtn = new QPushButton("🚀  게임 시작!");
    multiStartBtn->setFixedHeight(44);
    multiStartBtn->setEnabled(false);
    multiStartBtn->setStyleSheet(
        "QPushButton:enabled  { background: #e94560; font-size: 14px; }"
        "QPushButton:disabled { background: #222244; color: #555; font-size: 14px; }");
    layout->addWidget(multiStartBtn);

    layout->addStretch();

    // ── Tip ───────────────────────────────────────────────────────────────────
    auto* tip = new QLabel(
        "💡  같은 WiFi라면 호스트 PC의 내부 IP를 사용하세요.\n"
        "     외부 인터넷이라면 공유기 포트 포워딩이 필요합니다.");
    tip->setStyleSheet("color: #555577; font-size: 10px;");
    tip->setWordWrap(true);
    layout->addWidget(tip);

    tabs->addTab(w, "🌐  멀티");

    // ── Connections ───────────────────────────────────────────────────────────
    connect(hostRadio, &QRadioButton::toggled, [this](bool host) {
        connectBtn->setText(host ? "🌐  방 만들기" : "🔌  방 참가");
        ipEdit->setEnabled(!host);
    });
    connect(connectBtn,    &QPushButton::clicked, this, &MainWindow::onMultiConnect);
    connect(cancelBtn,     &QPushButton::clicked, this, &MainWindow::onMultiCancel);
    connect(multiStartBtn, &QPushButton::clicked, this, &MainWindow::onMultiStartGame);
}

// ─────────────────────────────────────────────────────────────────────────────
// Single-player slots
void MainWindow::onStartPause()
{
    // 네트워크 모드에서는 방장만 일시정지 가능
    if (netManager && netManager->isConnected() && !iAmHost) return;

    if (!game->isRunning() && !game->isPaused()) {
        QString name = nameEdit->text().trimmed();
        if (name.isEmpty()) name = "Anonymous";
        game->setDifficulty(difficultyCombo->currentIndex() + 1);
        game->startGame(name);
        startBtn->setText("⏸  일시정지");
        nameEdit->setEnabled(false);
        difficultyCombo->setEnabled(false);
    } else if (game->isRunning()) {
        game->pauseGame();
        startBtn->setText("▶  재개");
        // 클라이언트에게 일시정지 알림
        if (netManager && iAmHost) {
            QJsonObject m; m["t"] = "pause";
            netManager->sendJson(m);
        }
    } else {
        game->resumeGame();
        startBtn->setText("⏸  일시정지");
        // 클라이언트에게 재개 알림
        if (netManager && iAmHost) {
            QJsonObject m; m["t"] = "resume";
            netManager->sendJson(m);
        }
    }
}

void MainWindow::onGameOver(QString name, int score, int kills, int level, int playSec)
{
    Database::instance().saveRecord(name, score, kills, level, playSec);
    refreshRecords();

    if (netManager && netManager->isConnected()) {
        // 멀티 게임 종료 - 버튼 초기화
        startBtn->setText(iAmHost ? "▶  다시 시작" : "게임 종료됨");
        startBtn->setEnabled(iAmHost);
    } else {
        startBtn->setText("▶  다시 시작");
        startBtn->setEnabled(true);
    }
    nameEdit->setEnabled(true);
    difficultyCombo->setEnabled(true);
}

void MainWindow::onStatsChanged(int score, int lives, int level)
{
    scoreLabel->setText(QString("점수: %1").arg(score));
    levelLabel->setText(QString("레벨: %1").arg(level));
    QString h;
    for (int i = 0; i < lives; ++i) h += "♥ ";
    livesLabel->setText(h.isEmpty() ? "💀" : h.trimmed());
}

// ─────────────────────────────────────────────────────────────────────────────
// Records slots
void MainWindow::onSearchClicked()
{
    refreshRecords(searchName->text().trimmed(), fromDate->date(), toDate->date());
}

void MainWindow::onResetClicked()
{
    searchName->clear();
    fromDate->setDate(QDate::currentDate().addMonths(-1));
    toDate->setDate(QDate::currentDate());
    refreshRecords();
}

void MainWindow::refreshRecords(const QString& name, const QDate& from, const QDate& to)
{
    auto records = Database::instance().getRecords(name, from, to);
    recordTable->setRowCount(records.size());

    for (int i = 0; i < records.size(); ++i) {
        const auto& r = records[i];
        int sec = r.playTimeSec;
        QString timeStr = QString("%1:%2").arg(sec/60,2,10,QChar('0')).arg(sec%60,2,10,QChar('0'));

        auto setCell = [&](int col, const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            if      (i == 0) item->setForeground(QColor(255,215,0));
            else if (i == 1) item->setForeground(QColor(180,180,200));
            else if (i == 2) item->setForeground(QColor(180,110,60));
            recordTable->setItem(i, col, item);
        };

        QString rank = (i==0)?"🥇":(i==1)?"🥈":(i==2)?"🥉":QString("#%1").arg(i+1);
        setCell(0, rank);
        setCell(1, r.playerName);
        setCell(2, QString::number(r.score));
        setCell(3, QString::number(r.kills));
        setCell(4, QString::number(r.level));
        setCell(5, timeStr);
        setCell(6, r.dateTime.toString("yy-MM-dd hh:mm"));
    }

    for (int c : {0,2,3,4,5}) recordTable->resizeColumnToContents(c);
    summaryLabel->setText(QString("총 %1개의 기록  (점수 높은 순)").arg(records.size()));
}

void MainWindow::onMultiCancel()
{
    if (netManager) {
        netManager->closeAll();
        netManager->deleteLater();
        netManager = nullptr;
    }
    game->stopGame("멀티 연결이 종료되었습니다.\n\n다시 하려면 멀티 탭에서 연결하세요.");
    startBtn->setText("▶  시작");
    startBtn->setEnabled(true);
    nameEdit->setEnabled(true);
    difficultyCombo->setEnabled(true);
    netStatusLabel->setText("연결 취소됨");
    netStatusLabel->setStyleSheet("color: #888; border: 1px solid #333; border-radius:6px; padding:8px;");
    connectBtn->setEnabled(true);
    cancelBtn->setEnabled(false);
    multiStartBtn->setEnabled(false);
    playerListLabel->setText("(없음)");
    remotePlayerName.clear();
}

void MainWindow::onMultiConnect()
{
    // Tear down any previous session
    if (netManager) {
        netManager->closeAll();
        netManager->deleteLater();
        netManager = nullptr;
    }
    game->stopGame("멀티 연결 준비 중...\n\n상대방과 연결한 뒤 게임을 시작하세요.");
    startBtn->setText("▶  시작");
    startBtn->setEnabled(true);
    nameEdit->setEnabled(true);
    difficultyCombo->setEnabled(true);

    netManager = new NetworkManager(this);
    connect(netManager, &NetworkManager::peerConnected,    this, &MainWindow::onNetPeerConnected);
    connect(netManager, &NetworkManager::peerDisconnected, this, &MainWindow::onNetPeerDisconnected);
    connect(netManager, &NetworkManager::messageReceived,  this, &MainWindow::onNetMessage);
    connect(netManager, &NetworkManager::networkError,     this, &MainWindow::onNetError);

    quint16 port = (quint16)portEdit->text().toUShort();
    if (!port) port = 7777;

    iAmHost = hostRadio->isChecked();

    if (iAmHost) {
        if (netManager->host(port)) {
            netStatusLabel->setText(
                QString("🟡  서버 대기 중...\n포트 %1 에서 상대방을 기다리는 중입니다.").arg(port));
            netStatusLabel->setStyleSheet("color: #ffcc00; border: 1px solid #555; border-radius:6px; padding:10px; font-size:12px;");
            connectBtn->setEnabled(false);
            cancelBtn->setEnabled(true);
        }
    } else {
        QString ip = ipEdit->text().trimmed();
        if (ip.isEmpty()) ip = "127.0.0.1";
        netManager->join(ip, port);
        netStatusLabel->setText(QString("🟡  %1:%2 에 연결 중...\n\n연결 실패 시:\n"
                                        "① 호스트 IP/포트가 맞는지 확인\n"
                                        "② 호스트 PC 방화벽에서 포트 %2 허용").arg(ip).arg(port));
        netStatusLabel->setStyleSheet("color: #ffcc00; border: 1px solid #555; border-radius:6px; padding:10px; font-size:11px;");
        connectBtn->setEnabled(false);
        cancelBtn->setEnabled(true);
    }
}

void MainWindow::onNetPeerConnected()
{
    // Exchange names
    QString myName = multiNameEdit->text().trimmed();
    if (myName.isEmpty()) myName = "Player";
    QJsonObject hello; hello["t"] = "hello"; hello["name"] = myName;
    netManager->sendJson(hello);

    netStatusLabel->setText("🟢  연결 성공! 상대방과 연결되었습니다.");
    netStatusLabel->setStyleSheet("color: #00ff88; border: 1px solid #555; border-radius:6px; padding:8px;");

    if (iAmHost) {
        multiStartBtn->setEnabled(true);
    }
}

void MainWindow::onNetPeerDisconnected()
{
    game->stopGame("상대방 연결이 끊겼습니다.\n\n멀티 탭에서 다시 연결하세요.");
    startBtn->setText("▶  시작");
    startBtn->setEnabled(true);
    nameEdit->setEnabled(true);
    difficultyCombo->setEnabled(true);
    netStatusLabel->setText("🔴  상대방 연결 끊김");
    netStatusLabel->setStyleSheet("color: #ff5555; border: 1px solid #555; border-radius:6px; padding:8px;");
    multiStartBtn->setEnabled(false);
    connectBtn->setEnabled(true);
    cancelBtn->setEnabled(false);
    playerListLabel->setText("(없음)");
    remotePlayerName.clear();
}

void MainWindow::onNetError(QString err)
{
    if (netManager) {
        netManager->closeAll();
        netManager->deleteLater();
        netManager = nullptr;
    }
    game->stopGame("멀티 연결에 실패했습니다.\n\n설정을 확인하고 다시 연결하세요.");
    startBtn->setText("▶  시작");
    startBtn->setEnabled(true);
    nameEdit->setEnabled(true);
    difficultyCombo->setEnabled(true);
    netStatusLabel->setText(
        QString("❌  연결 실패: %1\n\n"
                "확인사항:\n"
                "① 호스트 IP / 포트가 정확한지\n"
                "② 호스트 PC 방화벽에서 해당 포트 허용\n"
                "③ 같은 네트워크(Wi-Fi)인지 확인").arg(err));
    netStatusLabel->setStyleSheet("color: #ff5555; border: 1px solid #555; border-radius:6px; padding:10px; font-size:11px;");
    connectBtn->setEnabled(true);
    cancelBtn->setEnabled(false);
    multiStartBtn->setEnabled(false);
    playerListLabel->setText("(없음)");
    remotePlayerName.clear();
}

// ─────────────────────────────────────────────────────────────────────────────
// Route incoming JSON – lobby messages handled here, game messages forwarded
void MainWindow::onNetMessage(QJsonObject obj)
{
    QString t = obj["t"].toString();

    if (t == "hello") {
        remotePlayerName = obj["name"].toString();
        QString myName   = multiNameEdit->text().trimmed();
        playerListLabel->setText(
            QString("🔵  %1  (나)\n🟢  %2  (상대방)").arg(myName, remotePlayerName));

        if (!iAmHost) {
            // Client waits for "start" from host
            netStatusLabel->setText(
                QString("🟢  연결됨 – %1 vs %2\n호스트가 시작할 때까지 대기 중...").arg(myName, remotePlayerName));
        }
    }
    else if (t == "start") {
        int difficulty = qBound(1, obj["difficulty"].toInt(1), 3);
        difficultyCombo->setCurrentIndex(difficulty - 1);
        // Host signaled start → client kicks off the game
        startNetworkGameNow();
    }
    else {
        // In-game message → forward to game widget
        game->handleNetMessage(obj);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Host clicked "게임 시작!"
void MainWindow::onMultiStartGame()
{
    QJsonObject msg; msg["t"] = "start";
    msg["difficulty"] = difficultyCombo->currentIndex() + 1;
    netManager->sendJson(msg);
    multiStartBtn->setEnabled(false);
    startNetworkGameNow();
}

// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::startNetworkGameNow()
{
    QString myName = multiNameEdit->text().trimmed();
    if (myName.isEmpty())            myName           = "Player1";
    if (remotePlayerName.isEmpty())  remotePlayerName = "Player2";

    nameEdit->setText(myName);
    nameEdit->setEnabled(false);
    difficultyCombo->setEnabled(false);

    // 클라이언트는 일시정지 버튼 비활성화
    if (iAmHost) {
        startBtn->setText("⏸  일시정지");
        startBtn->setEnabled(true);
    } else {
        startBtn->setText("방장만 일시정지 가능");
        startBtn->setEnabled(false);
    }

    game->setDifficulty(difficultyCombo->currentIndex() + 1);
    game->startNetworkGame(iAmHost, netManager, myName, remotePlayerName);

    // 탭 전환 후 포커스 설정 (딜레이로 위젯이 표시된 뒤에 적용)
    tabs->setCurrentIndex(0);
    QTimer::singleShot(100, this, [this]() { game->setFocus(); });
}
