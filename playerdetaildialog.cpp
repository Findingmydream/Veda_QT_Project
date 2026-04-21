#include "playerdetaildialog.h"
#include "titles.h"
#include "recordstable.h"
#include "playersearchdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTabWidget>
#include <QPixmap>
#include <QFileInfo>
#include <QSet>
#include <QStringList>
#include <QMessageBox>

PlayerDetailDialog::PlayerDetailDialog(const Player& player, QWidget* parent)
    : QDialog(parent)
    , player_(player)
{
    setWindowTitle(QString("플레이어 상세 - %1").arg(player_.nickname));
    setModal(true);
    resize(640, 600);

    auto* root = new QVBoxLayout(this);
    root->setSpacing(12);

    // === 헤더: 사진 + 닉/칭호 + 전적 요약 ===
    auto* headerRow = new QHBoxLayout;

    auto* avatar = new QLabel;
    avatar->setFixedSize(140, 140);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet("border:1px solid #444; background:#0d1117; color:#888;");
    if (!player_.avatarPath.isEmpty() && QFileInfo::exists(player_.avatarPath)) {
        QPixmap pix(player_.avatarPath);
        if (!pix.isNull()) {
            avatar->setPixmap(pix.scaled(avatar->size(),
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation));
        } else {
            avatar->setText("(사진 없음)");
        }
    } else {
        avatar->setText("(사진 없음)");
    }
    headerRow->addWidget(avatar, 0, Qt::AlignTop);

    auto* infoCol = new QVBoxLayout;
    auto* nickLabel = new QLabel(player_.nickname);
    nickLabel->setStyleSheet("color:white; font-size:20px; font-weight:bold;");
    infoCol->addWidget(nickLabel);

    auto* titleLabel = new QLabel(playerTitle(player_.totalGames(), player_.winRate()));
    titleLabel->setStyleSheet("color:#ffcc66; font-size:14px;");
    infoCol->addWidget(titleLabel);

    auto* statsBox = new QGroupBox("전적 요약");
    auto* statsForm = new QFormLayout(statsBox);
    statsForm->addRow("총 게임 수:",  new QLabel(QString::number(player_.totalGames()) + "판"));
    statsForm->addRow("승 / 패 / 무:", new QLabel(QString("%1승 %2패 %3무")
                                                   .arg(player_.wins).arg(player_.losses).arg(player_.draws)));
    statsForm->addRow("승률:",        new QLabel(QString::number(player_.winRate(), 'f', 1) + "%"));
    double avg = Database::instance().averageMoves(player_.id);
    statsForm->addRow("평균 수:",     new QLabel(player_.totalGames() > 0
                                                  ? QString::number(avg, 'f', 1) + "수"
                                                  : QString("-")));
    infoCol->addWidget(statsBox);
    infoCol->addStretch(1);
    headerRow->addLayout(infoCol, 1);

    root->addLayout(headerRow);

    // === sub-tabs: 개인 전적 / 상대 전적 ===
    auto* statsTabs = new QTabWidget;

    // --- 개인 전적 ---
    auto* personalTab = new QWidget;
    auto* personalLay = new QVBoxLayout(personalTab);
    personalTable_ = new QTableWidget;
    RecordsTable::setupColumns(personalTable_, /*includePlayerColumn=*/false);
    personalLay->addWidget(personalTable_, 1);
    statsTabs->addTab(personalTab, "개인 전적");

    // --- 상대 전적 ---
    auto* oppTab = new QWidget;
    auto* oppLay = new QVBoxLayout(oppTab);

    auto* oppTopRow = new QHBoxLayout;
    opponentSearchBtn_ = new QPushButton("🔍  상대 검색");
    opponentNameLabel_ = new QLabel("(선택 안됨)");
    opponentNameLabel_->setStyleSheet("color:#cfd8ff; font-weight:bold; padding:0 8px;");
    oppTopRow->addWidget(opponentSearchBtn_);
    oppTopRow->addWidget(opponentNameLabel_);
    oppTopRow->addStretch(1);
    oppLay->addLayout(oppTopRow);

    auto* oppStatsBox = new QGroupBox("상대 전적 (head-to-head)");
    auto* oppStatsForm = new QFormLayout(oppStatsBox);
    opponentTotalLabel_   = new QLabel("0판");
    opponentWLDLabel_     = new QLabel("0승 0패 0무");
    opponentWinRateLabel_ = new QLabel("0.0%");
    oppStatsForm->addRow("총 게임 수:",  opponentTotalLabel_);
    oppStatsForm->addRow("승 / 패 / 무:", opponentWLDLabel_);
    oppStatsForm->addRow("승률:",        opponentWinRateLabel_);
    oppLay->addWidget(oppStatsBox);

    opponentTable_ = new QTableWidget;
    RecordsTable::setupColumns(opponentTable_, /*includePlayerColumn=*/false);
    oppLay->addWidget(opponentTable_, 1);

    statsTabs->addTab(oppTab, "상대 전적");

    root->addWidget(statsTabs, 1);

    // 닫기
    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch(1);
    auto* closeBtn = new QPushButton("닫기");
    btnRow->addWidget(closeBtn);
    root->addLayout(btnRow);

    connect(closeBtn,           &QPushButton::clicked, this, &QDialog::accept);
    connect(opponentSearchBtn_, &QPushButton::clicked, this, &PlayerDetailDialog::onOpponentSearch);

    // 초기 데이터
    auto records = Database::instance().readRecords(player_.id);
    RecordsTable::fill(personalTable_, records, /*includePlayerColumn=*/false);
}

void PlayerDetailDialog::onOpponentSearch()
{
    auto records = Database::instance().readRecords(player_.id);
    QSet<QString> oppsSet;
    for (const auto& r : records) {
        QString op = r.opponent.trimmed();
        if (!op.isEmpty()) oppsSet.insert(op);
    }
    QStringList opps(oppsSet.begin(), oppsSet.end());
    opps.sort();

    if (opps.isEmpty()) {
        QMessageBox::information(this, "알림", "이 플레이어의 대전 기록이 없습니다.");
        return;
    }

    PlayerSearchDialog dlg(opps, this);
    if (dlg.exec() != QDialog::Accepted) return;
    QString name = dlg.selectedName();
    opponentNameLabel_->setText(name);
    refreshOpponentTab(name);
}

void PlayerDetailDialog::refreshOpponentTab(const QString& opponent)
{
    if (opponent.isEmpty()) {
        opponentTotalLabel_->setText("0판");
        opponentWLDLabel_->setText("0승 0패 0무");
        opponentWinRateLabel_->setText("0.0%");
        opponentTable_->setRowCount(0);
        return;
    }

    auto records = Database::instance().readRecords(player_.id);
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

    opponentTotalLabel_->setText(QString::number(total) + "판");
    opponentWLDLabel_->setText(QString("%1승 %2패 %3무").arg(wins).arg(losses).arg(draws));
    opponentWinRateLabel_->setText(QString::number(rate, 'f', 1) + "%");

    RecordsTable::fill(opponentTable_, matched, /*includePlayerColumn=*/false);
}
