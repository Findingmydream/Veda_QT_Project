/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "gamewidget.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *centralLayout;
    QHBoxLayout *appHeaderLayout;
    QLabel *headerStatusLabel;
    QSpacerItem *appHeaderSpacer;
    QLabel *accountLabel;
    QPushButton *logoutBtn;
    QTabWidget *tabs;
    QWidget *gameTab;
    QVBoxLayout *gameTabLayout;
    QGroupBox *gameSettingGroup;
    QHBoxLayout *gameSettingLayout;
    QLabel *playerLabel;
    QComboBox *playerCombo;
    QRadioButton *firstRadio;
    QRadioButton *secondRadio;
    QPushButton *startSingleBtn;
    QPushButton *pauseBtn;
    QPushButton *rematchBtn;
    QPushButton *leaveRoomBtn;
    QLabel *turnTimerLabel;
    QLabel *statusLabel;
    GameWidget *game;
    QWidget *profileTab;
    QVBoxLayout *profileTabLayout;
    QGroupBox *playerManageGroup;
    QFormLayout *playerManageLayout;
    QLabel *nickLabel;
    QLineEdit *nickEdit;
    QLabel *commentLabel;
    QLineEdit *commentEdit;
    QHBoxLayout *profileButtonLayout;
    QPushButton *createBtn;
    QPushButton *updateBtn;
    QPushButton *deletePlayerBtn;
    QTableWidget *playerTable;
    QWidget *recordTab;
    QVBoxLayout *recordTabLayout;
    QGroupBox *recordSearchGroup;
    QHBoxLayout *recordSearchLayout;
    QLabel *recordPlayerLabel;
    QComboBox *recordPlayerCombo;
    QLabel *resultFilterLabel;
    QComboBox *resultFilterCombo;
    QPushButton *searchRecordBtn;
    QGroupBox *multiSummaryGroup;
    QVBoxLayout *multiSummaryLayout;
    QTableWidget *multiSummaryTable;
    QTableWidget *recordTable;
    QPushButton *deleteRecordBtn;
    QWidget *multiTab;
    QVBoxLayout *multiTabLayout;
    QGroupBox *modeGroup;
    QVBoxLayout *modeLayout;
    QRadioButton *hostRadio;
    QRadioButton *joinRadio;
    QGroupBox *connectSettingGroup;
    QFormLayout *connectSettingLayout;
    QLabel *myIpLabel;
    QLineEdit *myIpEdit;
    QLabel *multiPlayerLabel;
    QComboBox *multiPlayerCombo;
    QLabel *portLabel;
    QLineEdit *portEdit;
    QLabel *ipLabel;
    QLineEdit *ipEdit;
    QHBoxLayout *multiButtonLayout;
    QPushButton *connectBtn;
    QPushButton *cancelBtn;
    QLabel *netStatusLabel;
    QLabel *playerListLabel;
    QPushButton *multiStartBtn;
    QSpacerItem *multiVerticalSpacer;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(700, 720);
        MainWindow->setMinimumSize(QSize(700, 720));
        MainWindow->setStyleSheet(QString::fromUtf8("QMainWindow { background: #1a1a2e; }\n"
"QTabWidget::pane { border:1px solid #2a2a4a; background:#16213e; }\n"
"QTabBar::tab { background:#0f3460; color:#ccc; padding:8px 16px; font-weight:bold; }\n"
"QTabBar::tab:selected { background:#e94560; color:white; }\n"
"QTabBar::tab:hover { background:#1a4a80; }\n"
"QGroupBox { color:#8888aa; border:1px solid #2a2a4a; border-radius:6px; margin-top:10px; padding-top:6px; font-weight:bold; }\n"
"QGroupBox::title { subcontrol-origin:margin; left:10px; top:-2px; }\n"
"QPushButton { background:#0f3460; color:white; border:none; padding:6px 16px; border-radius:5px; font-weight:bold; }\n"
"QPushButton:hover { background:#e94560; }\n"
"QPushButton:pressed { background:#b03040; }\n"
"QPushButton:disabled { background:#222244; color:#555; }\n"
"QLineEdit, QComboBox { background:#0d1117; color:white; border:1px solid #2a2a4a; border-radius:4px; padding:5px 8px; }\n"
"QComboBox::drop-down { border:none; }\n"
"QComboBox QAbstractItemView { background:#0d1117; color:white; }\n"
"Q"
                        "TableWidget { background:#0d1117; color:white; gridline-color:#1e1e3a; border:1px solid #2a2a4a; selection-background-color:#0f3460; }\n"
"QTableWidget QHeaderView::section { background:#0f3460; color:#ddd; padding:5px; border:none; font-weight:bold; }\n"
"QTableWidget::item:alternate { background:#0e1525; }\n"
"QListWidget { background:#0d1117; color:white; border:1px solid #2a2a4a; }\n"
"QListWidget::item:selected { background:#0f3460; }\n"
"QLabel { color:white; }\n"
"QRadioButton { color:#ddd; }"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        centralLayout = new QVBoxLayout(centralwidget);
        centralLayout->setObjectName("centralLayout");
        centralLayout->setContentsMargins(0, 0, 0, 0);
        appHeaderLayout = new QHBoxLayout();
        appHeaderLayout->setSpacing(10);
        appHeaderLayout->setObjectName("appHeaderLayout");
        appHeaderLayout->setContentsMargins(12, 8, 12, 4);
        headerStatusLabel = new QLabel(centralwidget);
        headerStatusLabel->setObjectName("headerStatusLabel");
        headerStatusLabel->setStyleSheet(QString::fromUtf8("color:#9aa7d9; padding:4px 2px;"));
        headerStatusLabel->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignVCenter);

        appHeaderLayout->addWidget(headerStatusLabel);

        appHeaderSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        appHeaderLayout->addItem(appHeaderSpacer);

        accountLabel = new QLabel(centralwidget);
        accountLabel->setObjectName("accountLabel");
        accountLabel->setStyleSheet(QString::fromUtf8("color:#cfd8ff; font-weight:bold; padding:4px 2px;"));
        accountLabel->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        appHeaderLayout->addWidget(accountLabel);

        logoutBtn = new QPushButton(centralwidget);
        logoutBtn->setObjectName("logoutBtn");
        logoutBtn->setMinimumSize(QSize(88, 30));
        logoutBtn->setMaximumSize(QSize(120, 32));
        logoutBtn->setStyleSheet(QString::fromUtf8("QPushButton{background:#663333;color:white;} QPushButton:hover{background:#994444;}"));

        appHeaderLayout->addWidget(logoutBtn);


        centralLayout->addLayout(appHeaderLayout);

        tabs = new QTabWidget(centralwidget);
        tabs->setObjectName("tabs");
        gameTab = new QWidget();
        gameTab->setObjectName("gameTab");
        gameTabLayout = new QVBoxLayout(gameTab);
        gameTabLayout->setSpacing(8);
        gameTabLayout->setObjectName("gameTabLayout");
        gameTabLayout->setContentsMargins(10, 10, 10, 6);
        gameSettingGroup = new QGroupBox(gameTab);
        gameSettingGroup->setObjectName("gameSettingGroup");
        gameSettingLayout = new QHBoxLayout(gameSettingGroup);
        gameSettingLayout->setObjectName("gameSettingLayout");
        playerLabel = new QLabel(gameSettingGroup);
        playerLabel->setObjectName("playerLabel");

        gameSettingLayout->addWidget(playerLabel);

        playerCombo = new QComboBox(gameSettingGroup);
        playerCombo->setObjectName("playerCombo");

        gameSettingLayout->addWidget(playerCombo);

        firstRadio = new QRadioButton(gameSettingGroup);
        firstRadio->setObjectName("firstRadio");
        firstRadio->setChecked(true);

        gameSettingLayout->addWidget(firstRadio);

        secondRadio = new QRadioButton(gameSettingGroup);
        secondRadio->setObjectName("secondRadio");

        gameSettingLayout->addWidget(secondRadio);

        startSingleBtn = new QPushButton(gameSettingGroup);
        startSingleBtn->setObjectName("startSingleBtn");

        gameSettingLayout->addWidget(startSingleBtn);

        pauseBtn = new QPushButton(gameSettingGroup);
        pauseBtn->setObjectName("pauseBtn");
        pauseBtn->setEnabled(false);

        gameSettingLayout->addWidget(pauseBtn);

        rematchBtn = new QPushButton(gameSettingGroup);
        rematchBtn->setObjectName("rematchBtn");
        rematchBtn->setEnabled(false);

        gameSettingLayout->addWidget(rematchBtn);

        leaveRoomBtn = new QPushButton(gameSettingGroup);
        leaveRoomBtn->setObjectName("leaveRoomBtn");
        leaveRoomBtn->setEnabled(false);

        gameSettingLayout->addWidget(leaveRoomBtn);

        turnTimerLabel = new QLabel(gameSettingGroup);
        turnTimerLabel->setObjectName("turnTimerLabel");
        turnTimerLabel->setMinimumSize(QSize(74, 30));
        turnTimerLabel->setStyleSheet(QString::fromUtf8("color:#00ff88; font-size:16px; font-weight:bold; padding:3px 8px;"));
        turnTimerLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gameSettingLayout->addWidget(turnTimerLabel);


        gameTabLayout->addWidget(gameSettingGroup);

        statusLabel = new QLabel(gameTab);
        statusLabel->setObjectName("statusLabel");
        statusLabel->setStyleSheet(QString::fromUtf8("color:#aaa; font-size:11px;"));
        statusLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gameTabLayout->addWidget(statusLabel);

        game = new GameWidget(gameTab);
        game->setObjectName("game");

        gameTabLayout->addWidget(game, 0, Qt::AlignmentFlag::AlignHCenter);

        tabs->addTab(gameTab, QString());
        profileTab = new QWidget();
        profileTab->setObjectName("profileTab");
        profileTabLayout = new QVBoxLayout(profileTab);
        profileTabLayout->setSpacing(8);
        profileTabLayout->setObjectName("profileTabLayout");
        profileTabLayout->setContentsMargins(10, 10, 10, 10);
        playerManageGroup = new QGroupBox(profileTab);
        playerManageGroup->setObjectName("playerManageGroup");
        playerManageLayout = new QFormLayout(playerManageGroup);
        playerManageLayout->setObjectName("playerManageLayout");
        nickLabel = new QLabel(playerManageGroup);
        nickLabel->setObjectName("nickLabel");

        playerManageLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, nickLabel);

        nickEdit = new QLineEdit(playerManageGroup);
        nickEdit->setObjectName("nickEdit");

        playerManageLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, nickEdit);

        commentLabel = new QLabel(playerManageGroup);
        commentLabel->setObjectName("commentLabel");

        playerManageLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, commentLabel);

        commentEdit = new QLineEdit(playerManageGroup);
        commentEdit->setObjectName("commentEdit");

        playerManageLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, commentEdit);

        profileButtonLayout = new QHBoxLayout();
        profileButtonLayout->setObjectName("profileButtonLayout");
        createBtn = new QPushButton(playerManageGroup);
        createBtn->setObjectName("createBtn");

        profileButtonLayout->addWidget(createBtn);

        updateBtn = new QPushButton(playerManageGroup);
        updateBtn->setObjectName("updateBtn");

        profileButtonLayout->addWidget(updateBtn);

        deletePlayerBtn = new QPushButton(playerManageGroup);
        deletePlayerBtn->setObjectName("deletePlayerBtn");

        profileButtonLayout->addWidget(deletePlayerBtn);


        playerManageLayout->setLayout(2, QFormLayout::ItemRole::FieldRole, profileButtonLayout);


        profileTabLayout->addWidget(playerManageGroup);

        playerTable = new QTableWidget(profileTab);
        playerTable->setObjectName("playerTable");

        profileTabLayout->addWidget(playerTable);

        tabs->addTab(profileTab, QString());
        recordTab = new QWidget();
        recordTab->setObjectName("recordTab");
        recordTabLayout = new QVBoxLayout(recordTab);
        recordTabLayout->setSpacing(8);
        recordTabLayout->setObjectName("recordTabLayout");
        recordTabLayout->setContentsMargins(10, 10, 10, 10);
        recordSearchGroup = new QGroupBox(recordTab);
        recordSearchGroup->setObjectName("recordSearchGroup");
        recordSearchLayout = new QHBoxLayout(recordSearchGroup);
        recordSearchLayout->setObjectName("recordSearchLayout");
        recordPlayerLabel = new QLabel(recordSearchGroup);
        recordPlayerLabel->setObjectName("recordPlayerLabel");

        recordSearchLayout->addWidget(recordPlayerLabel);

        recordPlayerCombo = new QComboBox(recordSearchGroup);
        recordPlayerCombo->setObjectName("recordPlayerCombo");

        recordSearchLayout->addWidget(recordPlayerCombo);

        resultFilterLabel = new QLabel(recordSearchGroup);
        resultFilterLabel->setObjectName("resultFilterLabel");

        recordSearchLayout->addWidget(resultFilterLabel);

        resultFilterCombo = new QComboBox(recordSearchGroup);
        resultFilterCombo->setObjectName("resultFilterCombo");

        recordSearchLayout->addWidget(resultFilterCombo);

        searchRecordBtn = new QPushButton(recordSearchGroup);
        searchRecordBtn->setObjectName("searchRecordBtn");

        recordSearchLayout->addWidget(searchRecordBtn);


        recordTabLayout->addWidget(recordSearchGroup);

        multiSummaryGroup = new QGroupBox(recordTab);
        multiSummaryGroup->setObjectName("multiSummaryGroup");
        multiSummaryLayout = new QVBoxLayout(multiSummaryGroup);
        multiSummaryLayout->setObjectName("multiSummaryLayout");
        multiSummaryTable = new QTableWidget(multiSummaryGroup);
        multiSummaryTable->setObjectName("multiSummaryTable");
        multiSummaryTable->setMaximumSize(QSize(16777215, 170));

        multiSummaryLayout->addWidget(multiSummaryTable);


        recordTabLayout->addWidget(multiSummaryGroup);

        recordTable = new QTableWidget(recordTab);
        recordTable->setObjectName("recordTable");

        recordTabLayout->addWidget(recordTable);

        deleteRecordBtn = new QPushButton(recordTab);
        deleteRecordBtn->setObjectName("deleteRecordBtn");

        recordTabLayout->addWidget(deleteRecordBtn);

        tabs->addTab(recordTab, QString());
        multiTab = new QWidget();
        multiTab->setObjectName("multiTab");
        multiTabLayout = new QVBoxLayout(multiTab);
        multiTabLayout->setSpacing(10);
        multiTabLayout->setObjectName("multiTabLayout");
        multiTabLayout->setContentsMargins(14, 14, 14, 14);
        modeGroup = new QGroupBox(multiTab);
        modeGroup->setObjectName("modeGroup");
        modeLayout = new QVBoxLayout(modeGroup);
        modeLayout->setObjectName("modeLayout");
        hostRadio = new QRadioButton(modeGroup);
        hostRadio->setObjectName("hostRadio");
        hostRadio->setChecked(true);

        modeLayout->addWidget(hostRadio);

        joinRadio = new QRadioButton(modeGroup);
        joinRadio->setObjectName("joinRadio");

        modeLayout->addWidget(joinRadio);


        multiTabLayout->addWidget(modeGroup);

        connectSettingGroup = new QGroupBox(multiTab);
        connectSettingGroup->setObjectName("connectSettingGroup");
        connectSettingLayout = new QFormLayout(connectSettingGroup);
        connectSettingLayout->setObjectName("connectSettingLayout");
        myIpLabel = new QLabel(connectSettingGroup);
        myIpLabel->setObjectName("myIpLabel");

        connectSettingLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, myIpLabel);

        myIpEdit = new QLineEdit(connectSettingGroup);
        myIpEdit->setObjectName("myIpEdit");
        myIpEdit->setStyleSheet(QString::fromUtf8("background:#0a0a1a; color:#00ff88; border:1px solid #2a2a4a; border-radius:4px; padding:5px;"));
        myIpEdit->setReadOnly(true);

        connectSettingLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, myIpEdit);

        multiPlayerLabel = new QLabel(connectSettingGroup);
        multiPlayerLabel->setObjectName("multiPlayerLabel");

        connectSettingLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, multiPlayerLabel);

        multiPlayerCombo = new QComboBox(connectSettingGroup);
        multiPlayerCombo->setObjectName("multiPlayerCombo");

        connectSettingLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, multiPlayerCombo);

        portLabel = new QLabel(connectSettingGroup);
        portLabel->setObjectName("portLabel");

        connectSettingLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, portLabel);

        portEdit = new QLineEdit(connectSettingGroup);
        portEdit->setObjectName("portEdit");

        connectSettingLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, portEdit);

        ipLabel = new QLabel(connectSettingGroup);
        ipLabel->setObjectName("ipLabel");

        connectSettingLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, ipLabel);

        ipEdit = new QLineEdit(connectSettingGroup);
        ipEdit->setObjectName("ipEdit");
        ipEdit->setEnabled(false);

        connectSettingLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, ipEdit);


        multiTabLayout->addWidget(connectSettingGroup);

        multiButtonLayout = new QHBoxLayout();
        multiButtonLayout->setObjectName("multiButtonLayout");
        connectBtn = new QPushButton(multiTab);
        connectBtn->setObjectName("connectBtn");

        multiButtonLayout->addWidget(connectBtn);

        cancelBtn = new QPushButton(multiTab);
        cancelBtn->setObjectName("cancelBtn");
        cancelBtn->setEnabled(false);
        cancelBtn->setStyleSheet(QString::fromUtf8("QPushButton:enabled{background:#663333;color:white;} QPushButton:disabled{background:#222233;color:#555;}"));

        multiButtonLayout->addWidget(cancelBtn);


        multiTabLayout->addLayout(multiButtonLayout);

        netStatusLabel = new QLabel(multiTab);
        netStatusLabel->setObjectName("netStatusLabel");
        netStatusLabel->setStyleSheet(QString::fromUtf8("color:#888; border:1px solid #333; border-radius:6px; padding:8px;"));
        netStatusLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);
        netStatusLabel->setWordWrap(true);

        multiTabLayout->addWidget(netStatusLabel);

        playerListLabel = new QLabel(multiTab);
        playerListLabel->setObjectName("playerListLabel");
        playerListLabel->setStyleSheet(QString::fromUtf8("color:#aaa; padding:6px;"));
        playerListLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        multiTabLayout->addWidget(playerListLabel);

        multiStartBtn = new QPushButton(multiTab);
        multiStartBtn->setObjectName("multiStartBtn");
        multiStartBtn->setEnabled(false);
        multiStartBtn->setMinimumSize(QSize(0, 44));
        multiStartBtn->setStyleSheet(QString::fromUtf8("QPushButton:enabled{background:#e94560;font-size:14px;} QPushButton:disabled{background:#222244;color:#555;font-size:14px;}"));

        multiTabLayout->addWidget(multiStartBtn);

        multiVerticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        multiTabLayout->addItem(multiVerticalSpacer);

        tabs->addTab(multiTab, QString());

        centralLayout->addWidget(tabs);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        tabs->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "\354\230\244\353\252\251 \352\262\214\354\236\204", nullptr));
        headerStatusLabel->setText(QCoreApplication::translate("MainWindow", "\352\262\214\354\236\204 \353\214\200\352\270\260 \354\244\221", nullptr));
        accountLabel->setText(QCoreApplication::translate("MainWindow", "\353\241\234\352\267\270\354\235\270: -", nullptr));
        logoutBtn->setText(QCoreApplication::translate("MainWindow", "\353\241\234\352\267\270\354\225\204\354\233\203", nullptr));
        gameSettingGroup->setTitle(QCoreApplication::translate("MainWindow", "\352\262\214\354\236\204 \354\204\244\354\240\225", nullptr));
        playerLabel->setText(QCoreApplication::translate("MainWindow", "\355\224\214\353\240\210\354\235\264\354\226\264:", nullptr));
        firstRadio->setText(QCoreApplication::translate("MainWindow", "\355\235\221 (\354\204\240\352\263\265)", nullptr));
        secondRadio->setText(QCoreApplication::translate("MainWindow", "\353\260\261 (\355\233\204\352\263\265)", nullptr));
        startSingleBtn->setText(QCoreApplication::translate("MainWindow", "\342\226\266  AI \353\214\200\354\240\204 \354\213\234\354\236\221", nullptr));
        pauseBtn->setText(QCoreApplication::translate("MainWindow", "\342\217\270  \354\235\274\354\213\234\354\240\225\354\247\200", nullptr));
        rematchBtn->setText(QCoreApplication::translate("MainWindow", "\342\206\273  \354\236\254\353\217\204\354\240\204", nullptr));
        leaveRoomBtn->setText(QCoreApplication::translate("MainWindow", "\360\237\232\252  \353\260\251 \353\202\230\352\260\200\352\270\260", nullptr));
        turnTimerLabel->setText(QCoreApplication::translate("MainWindow", "30\354\264\210", nullptr));
        statusLabel->setText(QCoreApplication::translate("MainWindow", "\355\224\214\353\240\210\354\235\264\354\226\264\353\245\274 \354\204\240\355\203\235\355\225\230\352\263\240 \352\262\214\354\236\204\354\235\204 \354\213\234\354\236\221\355\225\230\354\204\270\354\232\224.", nullptr));
        tabs->setTabText(tabs->indexOf(gameTab), QCoreApplication::translate("MainWindow", "\342\232\253  \352\262\214\354\236\204", nullptr));
        playerManageGroup->setTitle(QCoreApplication::translate("MainWindow", "\355\224\214\353\240\210\354\235\264\354\226\264 \352\264\200\353\246\254", nullptr));
        nickLabel->setText(QCoreApplication::translate("MainWindow", "\353\213\211\353\204\244\354\236\204 :", nullptr));
        nickEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "\353\213\211\353\204\244\354\236\204 \354\236\205\353\240\245", nullptr));
        commentLabel->setText(QCoreApplication::translate("MainWindow", "\355\225\234\353\247\210\353\224\224 :", nullptr));
        commentEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "\355\225\234\353\247\210\353\224\224 (\354\204\240\355\203\235)", nullptr));
        createBtn->setText(QCoreApplication::translate("MainWindow", "\342\236\225  \354\266\224\352\260\200", nullptr));
        updateBtn->setText(QCoreApplication::translate("MainWindow", "\342\234\217\357\270\217  \353\202\264 \355\224\204\353\241\234\355\225\204 \354\210\230\354\240\225", nullptr));
        deletePlayerBtn->setText(QCoreApplication::translate("MainWindow", "\360\237\227\221  \354\202\255\354\240\234", nullptr));
        tabs->setTabText(tabs->indexOf(profileTab), QCoreApplication::translate("MainWindow", "\360\237\221\244  \355\224\204\353\241\234\355\225\204", nullptr));
        recordSearchGroup->setTitle(QCoreApplication::translate("MainWindow", "\360\237\224\215  \352\262\200\354\203\211", nullptr));
        recordPlayerLabel->setText(QCoreApplication::translate("MainWindow", "\355\224\214\353\240\210\354\235\264\354\226\264:", nullptr));
        resultFilterLabel->setText(QCoreApplication::translate("MainWindow", "\352\262\260\352\263\274:", nullptr));
        searchRecordBtn->setText(QCoreApplication::translate("MainWindow", "\352\262\200\354\203\211", nullptr));
        multiSummaryGroup->setTitle(QCoreApplication::translate("MainWindow", "\353\251\200\355\213\260 \354\203\201\353\214\200\353\263\204 \354\240\204\354\240\201", nullptr));
        deleteRecordBtn->setText(QCoreApplication::translate("MainWindow", "\360\237\227\221  \354\204\240\355\203\235 \352\270\260\353\241\235 \354\202\255\354\240\234", nullptr));
        tabs->setTabText(tabs->indexOf(recordTab), QCoreApplication::translate("MainWindow", "\360\237\223\212  \354\240\204\354\240\201", nullptr));
        modeGroup->setTitle(QCoreApplication::translate("MainWindow", "\353\252\250\353\223\234", nullptr));
        hostRadio->setText(QCoreApplication::translate("MainWindow", "\360\237\217\240  \353\260\251 \353\247\214\353\223\244\352\270\260 (Host)", nullptr));
        joinRadio->setText(QCoreApplication::translate("MainWindow", "\360\237\224\214  \353\260\251 \354\260\270\352\260\200 (Join)", nullptr));
        connectSettingGroup->setTitle(QCoreApplication::translate("MainWindow", "\354\240\221\354\206\215 \354\204\244\354\240\225", nullptr));
        myIpLabel->setText(QCoreApplication::translate("MainWindow", "\353\202\264 IP :", nullptr));
        multiPlayerLabel->setText(QCoreApplication::translate("MainWindow", "\355\224\214\353\240\210\354\235\264\354\226\264 :", nullptr));
        portLabel->setText(QCoreApplication::translate("MainWindow", "\355\217\254\355\212\270 :", nullptr));
        portEdit->setText(QCoreApplication::translate("MainWindow", "7777", nullptr));
        ipLabel->setText(QCoreApplication::translate("MainWindow", "\355\230\270\354\212\244\355\212\270 IP :", nullptr));
        ipEdit->setText(QCoreApplication::translate("MainWindow", "127.0.0.1", nullptr));
        ipEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "\355\230\270\354\212\244\355\212\270 IP \354\236\205\353\240\245", nullptr));
        connectBtn->setText(QCoreApplication::translate("MainWindow", "\360\237\214\220  \353\260\251 \353\247\214\353\223\244\352\270\260", nullptr));
        cancelBtn->setText(QCoreApplication::translate("MainWindow", "\342\234\225  \354\267\250\354\206\214", nullptr));
        netStatusLabel->setText(QCoreApplication::translate("MainWindow", "\354\227\260\352\262\260\353\220\230\354\247\200 \354\225\212\354\235\214", nullptr));
        playerListLabel->setText(QCoreApplication::translate("MainWindow", "(\354\227\206\354\235\214)", nullptr));
        multiStartBtn->setText(QCoreApplication::translate("MainWindow", "\360\237\232\200  \352\262\214\354\236\204 \354\213\234\354\236\221!", nullptr));
        tabs->setTabText(tabs->indexOf(multiTab), QCoreApplication::translate("MainWindow", "\360\237\214\220  \353\251\200\355\213\260", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
