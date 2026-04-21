#include "playerdetaildialog.h"
#include "titles.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QPixmap>
#include <QFileInfo>

PlayerDetailDialog::PlayerDetailDialog(const Player& player, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QString("플레이어 상세 - %1").arg(player.nickname));
    setModal(true);
    resize(560, 520);

    auto* root = new QVBoxLayout(this);
    root->setSpacing(12);

    // 헤더: 사진 + 닉/칭호
    auto* headerRow = new QHBoxLayout;

    auto* avatar = new QLabel;
    avatar->setFixedSize(140, 140);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet("border:1px solid #444; background:#0d1117; color:#888;");
    if (!player.avatarPath.isEmpty() && QFileInfo::exists(player.avatarPath)) {
        QPixmap pix(player.avatarPath);
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
    auto* nickLabel = new QLabel(player.nickname);
    nickLabel->setStyleSheet("color:white; font-size:20px; font-weight:bold;");
    infoCol->addWidget(nickLabel);

    auto* titleLabel = new QLabel(playerTitle(player.totalGames(), player.winRate()));
    titleLabel->setStyleSheet("color:#ffcc66; font-size:14px;");
    infoCol->addWidget(titleLabel);

    auto* statsBox = new QGroupBox("전적");
    auto* statsForm = new QFormLayout(statsBox);
    statsForm->addRow("총 게임 수:",  new QLabel(QString::number(player.totalGames()) + "판"));
    statsForm->addRow("승 / 패 / 무:", new QLabel(QString("%1승 %2패 %3무")
                                                   .arg(player.wins).arg(player.losses).arg(player.draws)));
    statsForm->addRow("승률:",        new QLabel(QString::number(player.winRate(), 'f', 1) + "%"));
    double avg = Database::instance().averageMoves(player.id);
    statsForm->addRow("평균 수:",     new QLabel(player.totalGames() > 0
                                                  ? QString::number(avg, 'f', 1) + "수"
                                                  : QString("-")));
    infoCol->addWidget(statsBox);
    infoCol->addStretch(1);
    headerRow->addLayout(infoCol, 1);

    root->addLayout(headerRow);

    // 기록 테이블
    auto* recordsBox = new QGroupBox("게임 기록");
    auto* recordsLay = new QVBoxLayout(recordsBox);
    auto* table = new QTableWidget;
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"상대","결과","수","날짜"});
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setStretchLastSection(true);

    auto records = Database::instance().readRecords(player.id);
    table->setRowCount(records.size());
    for (int i = 0; i < records.size(); ++i) {
        const auto& r = records[i];
        auto setC = [&](int col, const QString& t, QColor color = Qt::white) {
            auto* it = new QTableWidgetItem(t);
            it->setTextAlignment(Qt::AlignCenter);
            it->setForeground(color);
            table->setItem(i, col, it);
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
    recordsLay->addWidget(table);
    root->addWidget(recordsBox, 1);

    // 닫기 버튼
    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch(1);
    auto* closeBtn = new QPushButton("닫기");
    btnRow->addWidget(closeBtn);
    root->addLayout(btnRow);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}
