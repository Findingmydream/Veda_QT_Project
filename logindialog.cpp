#include "logindialog.h"
#include "ui_logindialog.h"

#include <QDebug>
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    // 한마디 기능 제거 — 회원가입 폼에서 숨김
    ui->registerCommentLabel->hide();
    ui->registerCommentEdit->hide();

    connect(ui->loginBtn, &QPushButton::clicked, this, &LoginDialog::onLogin);
    connect(ui->registerBtn, &QPushButton::clicked, this, &LoginDialog::onRegister);
    connect(ui->loginPasswordEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLogin);
    connect(ui->registerConfirmEdit, &QLineEdit::returnPressed, this, &LoginDialog::onRegister);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

Player LoginDialog::loggedInPlayer() const
{
    return loggedIn_;
}

void LoginDialog::onLogin()
{
    QString nick = ui->loginNickEdit->text().trimmed();
    QString password = ui->loginPasswordEdit->text();

    if (nick.isEmpty() || password.isEmpty()) {
        qInfo() << "[Auth] Login blocked: missing nickname or password";
        ui->loginMessageLabel->setText("닉네임과 비밀번호를 입력하세요.");
        return;
    }

    Player player;
    if (!Database::instance().authenticatePlayer(nick, password, &player)) {
        qInfo() << "[Auth] Login failed for user:" << nick;
        ui->loginMessageLabel->setText("닉네임 또는 비밀번호가 맞지 않습니다.");
        return;
    }

    loggedIn_ = player;
    qInfo() << "[Auth] Login success:" << loggedIn_.nickname << "(id:" << loggedIn_.id << ")";
    accept();
}

void LoginDialog::onRegister()
{
    QString nick = ui->registerNickEdit->text().trimmed();
    QString password = ui->registerPasswordEdit->text();
    QString confirm = ui->registerConfirmEdit->text();
    QString comment;

    if (nick.isEmpty()) {
        qInfo() << "[Auth] Register blocked: missing nickname";
        ui->registerMessageLabel->setText("닉네임을 입력하세요.");
        return;
    }
    if (password.size() < 4) {
        qInfo() << "[Auth] Register blocked for user:" << nick << "- password too short";
        ui->registerMessageLabel->setText("비밀번호는 4자 이상 입력하세요.");
        return;
    }
    if (password != confirm) {
        qInfo() << "[Auth] Register blocked for user:" << nick << "- password confirmation mismatch";
        ui->registerMessageLabel->setText("비밀번호 확인이 일치하지 않습니다.");
        return;
    }
    if (Database::instance().playerExists(nick)) {
        Player existing = Database::instance().readPlayerByName(nick);
        if (existing.hasPassword) {
            qInfo() << "[Auth] Register failed: duplicate user:" << nick;
            ui->registerMessageLabel->setText("이미 사용 중인 닉네임입니다.");
            return;
        }
        if (!Database::instance().setPlayerPassword(existing.id, password)) {
            qInfo() << "[Auth] Existing profile password registration failed:" << nick
                    << "(id:" << existing.id << ")";
            ui->registerMessageLabel->setText("기존 프로필에 비밀번호 등록을 실패했습니다.");
            return;
        }
        if (!comment.isEmpty())
            Database::instance().updatePlayer(existing.id, nick, comment);
        qInfo() << "[Auth] Existing profile converted to account:" << nick
                << "(id:" << existing.id << ")";
    } else {
        if (!Database::instance().createPlayerAccount(nick, password, comment)) {
            qInfo() << "[Auth] Register failed: database insert error for user:" << nick;
            ui->registerMessageLabel->setText("회원가입에 실패했습니다.");
            return;
        }
        Player created = Database::instance().readPlayerByName(nick);
        qInfo() << "[Auth] Register success:" << nick << "(id:" << created.id << ")";
    }

    if (!Database::instance().authenticatePlayer(nick, password, &loggedIn_)) {
        qInfo() << "[Auth] Auto-login failed after register:" << nick;
        QMessageBox::warning(this, "알림", "계정은 생성됐지만 자동 로그인에 실패했습니다.");
        return;
    }

    qInfo() << "[Auth] Auto-login success after register:" << loggedIn_.nickname
            << "(id:" << loggedIn_.id << ")";
    accept();
}
