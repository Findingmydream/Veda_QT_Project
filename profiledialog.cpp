#include "profiledialog.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QFileInfo>

ProfileDialog::ProfileDialog(Mode mode, const Player& current, QWidget* parent)
    : QDialog(parent)
    , mode_(mode)
    , original_(current)
    , avatarPath_(current.avatarPath)
{
    setWindowTitle(mode == Add ? "프로필 추가" : "프로필 수정");
    setModal(true);
    setFixedSize(360, 420);

    auto* root = new QVBoxLayout(this);

    // 사진 미리보기
    avatarPreview_ = new QLabel;
    avatarPreview_->setFixedSize(120, 120);
    avatarPreview_->setAlignment(Qt::AlignCenter);
    avatarPreview_->setStyleSheet("border:1px solid #555; background:#0d1117; color:#888;");
    root->addWidget(avatarPreview_, 0, Qt::AlignHCenter);

    pickAvatarBtn_ = new QPushButton("📷  사진 선택");
    root->addWidget(pickAvatarBtn_, 0, Qt::AlignHCenter);

    // 입력 폼
    auto* form = new QFormLayout;
    nickEdit_ = new QLineEdit(current.nickname);
    form->addRow("닉네임 :", nickEdit_);

    if (mode == Add) {
        passwordEdit_ = new QLineEdit;
        passwordEdit_->setEchoMode(QLineEdit::Password);
        passwordEdit_->setPlaceholderText("4자 이상");
        passwordConfirmEdit_ = new QLineEdit;
        passwordConfirmEdit_->setEchoMode(QLineEdit::Password);
        form->addRow("비밀번호 :",   passwordEdit_);
        form->addRow("비밀번호 확인 :", passwordConfirmEdit_);
    }
    root->addLayout(form);

    messageLabel_ = new QLabel;
    messageLabel_->setStyleSheet("color:#ff6666;");
    messageLabel_->setWordWrap(true);
    root->addWidget(messageLabel_);

    root->addStretch(1);

    // 버튼
    auto* btnRow = new QHBoxLayout;
    cancelBtn_ = new QPushButton("취소");
    saveBtn_   = new QPushButton(mode == Add ? "추가" : "저장");
    btnRow->addStretch(1);
    btnRow->addWidget(cancelBtn_);
    btnRow->addWidget(saveBtn_);
    root->addLayout(btnRow);

    connect(pickAvatarBtn_, &QPushButton::clicked, this, &ProfileDialog::onPickAvatar);
    connect(saveBtn_,       &QPushButton::clicked, this, &ProfileDialog::onSave);
    connect(cancelBtn_,     &QPushButton::clicked, this, &QDialog::reject);

    updateAvatarPreview();
}

QString ProfileDialog::nickname() const  { return nickEdit_->text().trimmed(); }
QString ProfileDialog::password() const  { return passwordEdit_ ? passwordEdit_->text() : QString(); }
QString ProfileDialog::avatarPath() const { return avatarPath_; }

void ProfileDialog::onPickAvatar()
{
    QString path = QFileDialog::getOpenFileName(
        this, "프로필 사진 선택", QString(),
        "이미지 (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (path.isEmpty()) return;
    avatarPath_ = path;
    updateAvatarPreview();
}

void ProfileDialog::updateAvatarPreview()
{
    if (avatarPath_.isEmpty() || !QFileInfo::exists(avatarPath_)) {
        avatarPreview_->setText("(사진 없음)");
        avatarPreview_->setPixmap(QPixmap());
        return;
    }
    QPixmap pix(avatarPath_);
    if (pix.isNull()) {
        avatarPreview_->setText("(불러올 수 없음)");
        return;
    }
    avatarPreview_->setPixmap(pix.scaled(avatarPreview_->size(),
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation));
}

void ProfileDialog::onSave()
{
    QString nick = nickname();
    if (nick.isEmpty()) {
        messageLabel_->setText("닉네임을 입력하세요.");
        return;
    }

    if (mode_ == Add) {
        QString pw = passwordEdit_->text();
        QString pw2 = passwordConfirmEdit_->text();
        if (pw.size() < 4) {
            messageLabel_->setText("비밀번호는 4자 이상 입력하세요.");
            return;
        }
        if (pw != pw2) {
            messageLabel_->setText("비밀번호 확인이 일치하지 않습니다.");
            return;
        }
        if (Database::instance().playerExists(nick)) {
            messageLabel_->setText("이미 사용 중인 닉네임입니다.");
            return;
        }
    } else { // Edit
        if (nick != original_.nickname && Database::instance().playerExists(nick)) {
            messageLabel_->setText("이미 사용 중인 닉네임입니다.");
            return;
        }
        if (QMessageBox::question(this, "수정 확인",
                                  "정말 수정하시겠습니까?",
                                  QMessageBox::Yes | QMessageBox::No)
            != QMessageBox::Yes) {
            return;
        }
    }

    accept();
}
