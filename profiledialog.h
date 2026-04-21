#pragma once
#include <QDialog>
#include "database.h"

class QLineEdit;
class QLabel;
class QPushButton;

class ProfileDialog : public QDialog
{
    Q_OBJECT
public:
    enum Mode { Add, Edit };

    explicit ProfileDialog(Mode mode, const Player& current, QWidget* parent = nullptr);

    QString nickname() const;
    QString password() const;     // Add 모드 전용
    QString avatarPath() const;

private slots:
    void onPickAvatar();
    void onSave();

private:
    void updateAvatarPreview();

    Mode      mode_;
    Player    original_;
    QString   avatarPath_;

    QLineEdit*   nickEdit_      = nullptr;
    QLineEdit*   passwordEdit_  = nullptr;
    QLineEdit*   passwordConfirmEdit_ = nullptr;
    QLabel*      avatarPreview_ = nullptr;
    QPushButton* pickAvatarBtn_ = nullptr;
    QPushButton* saveBtn_       = nullptr;
    QPushButton* cancelBtn_     = nullptr;
    QLabel*      messageLabel_  = nullptr;
};
