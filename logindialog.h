#pragma once

#include <QDialog>
#include "database.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QWidget* parent = nullptr);
    ~LoginDialog();

    Player loggedInPlayer() const;

private slots:
    void onLogin();
    void onRegister();

private:
    Player loggedIn_;
    Ui::LoginDialog* ui = nullptr;
};
