/********************************************************************************
** Form generated from reading UI file 'logindialog.ui'
**
** Created by: Qt User Interface Compiler version 6.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGINDIALOG_H
#define UI_LOGINDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LoginDialog
{
public:
    QVBoxLayout *rootLayout;
    QLabel *titleLabel;
    QTabWidget *tabs;
    QWidget *loginPage;
    QVBoxLayout *loginLayout;
    QFormLayout *loginFormLayout;
    QLabel *loginNickLabel;
    QLineEdit *loginNickEdit;
    QLabel *loginPasswordLabel;
    QLineEdit *loginPasswordEdit;
    QLabel *loginMessageLabel;
    QPushButton *loginBtn;
    QSpacerItem *loginVerticalSpacer;
    QWidget *registerPage;
    QVBoxLayout *registerLayout;
    QFormLayout *registerFormLayout;
    QLabel *registerNickLabel;
    QLineEdit *registerNickEdit;
    QLabel *registerPasswordLabel;
    QLineEdit *registerPasswordEdit;
    QLabel *registerConfirmLabel;
    QLineEdit *registerConfirmEdit;
    QLabel *registerCommentLabel;
    QLineEdit *registerCommentEdit;
    QLabel *registerMessageLabel;
    QPushButton *registerBtn;
    QSpacerItem *registerVerticalSpacer;

    void setupUi(QDialog *LoginDialog)
    {
        if (LoginDialog->objectName().isEmpty())
            LoginDialog->setObjectName("LoginDialog");
        LoginDialog->resize(380, 387);
        LoginDialog->setMinimumSize(QSize(380, 0));
        LoginDialog->setStyleSheet(QString::fromUtf8("QDialog { background:#1a1a2e; }\n"
"QTabWidget::pane { border:1px solid #2a2a4a; background:#16213e; }\n"
"QTabBar::tab { background:#0f3460; color:#ccc; padding:8px 18px; font-weight:bold; }\n"
"QTabBar::tab:selected { background:#e94560; color:white; }\n"
"QLabel { color:white; }\n"
"QLabel#loginMessageLabel, QLabel#registerMessageLabel { color:#ffcc66; min-height:22px; }\n"
"QLineEdit { background:#0d1117; color:white; border:1px solid #2a2a4a; border-radius:4px; padding:7px 8px; }\n"
"QPushButton { background:#0f3460; color:white; border:none; padding:8px 16px; border-radius:5px; font-weight:bold; }\n"
"QPushButton:hover { background:#e94560; }"));
        LoginDialog->setModal(true);
        rootLayout = new QVBoxLayout(LoginDialog);
        rootLayout->setObjectName("rootLayout");
        titleLabel = new QLabel(LoginDialog);
        titleLabel->setObjectName("titleLabel");
        titleLabel->setStyleSheet(QString::fromUtf8("font-size:22px; font-weight:bold; margin:8px 0 4px;"));
        titleLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        rootLayout->addWidget(titleLabel);

        tabs = new QTabWidget(LoginDialog);
        tabs->setObjectName("tabs");
        loginPage = new QWidget();
        loginPage->setObjectName("loginPage");
        loginLayout = new QVBoxLayout(loginPage);
        loginLayout->setObjectName("loginLayout");
        loginFormLayout = new QFormLayout();
        loginFormLayout->setObjectName("loginFormLayout");
        loginNickLabel = new QLabel(loginPage);
        loginNickLabel->setObjectName("loginNickLabel");

        loginFormLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, loginNickLabel);

        loginNickEdit = new QLineEdit(loginPage);
        loginNickEdit->setObjectName("loginNickEdit");

        loginFormLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, loginNickEdit);

        loginPasswordLabel = new QLabel(loginPage);
        loginPasswordLabel->setObjectName("loginPasswordLabel");

        loginFormLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, loginPasswordLabel);

        loginPasswordEdit = new QLineEdit(loginPage);
        loginPasswordEdit->setObjectName("loginPasswordEdit");
        loginPasswordEdit->setEchoMode(QLineEdit::EchoMode::Password);

        loginFormLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, loginPasswordEdit);


        loginLayout->addLayout(loginFormLayout);

        loginMessageLabel = new QLabel(loginPage);
        loginMessageLabel->setObjectName("loginMessageLabel");
        loginMessageLabel->setWordWrap(true);

        loginLayout->addWidget(loginMessageLabel);

        loginBtn = new QPushButton(loginPage);
        loginBtn->setObjectName("loginBtn");

        loginLayout->addWidget(loginBtn);

        loginVerticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        loginLayout->addItem(loginVerticalSpacer);

        tabs->addTab(loginPage, QString());
        registerPage = new QWidget();
        registerPage->setObjectName("registerPage");
        registerLayout = new QVBoxLayout(registerPage);
        registerLayout->setObjectName("registerLayout");
        registerFormLayout = new QFormLayout();
        registerFormLayout->setObjectName("registerFormLayout");
        registerNickLabel = new QLabel(registerPage);
        registerNickLabel->setObjectName("registerNickLabel");

        registerFormLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, registerNickLabel);

        registerNickEdit = new QLineEdit(registerPage);
        registerNickEdit->setObjectName("registerNickEdit");

        registerFormLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, registerNickEdit);

        registerPasswordLabel = new QLabel(registerPage);
        registerPasswordLabel->setObjectName("registerPasswordLabel");

        registerFormLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, registerPasswordLabel);

        registerPasswordEdit = new QLineEdit(registerPage);
        registerPasswordEdit->setObjectName("registerPasswordEdit");
        registerPasswordEdit->setEchoMode(QLineEdit::EchoMode::Password);

        registerFormLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, registerPasswordEdit);

        registerConfirmLabel = new QLabel(registerPage);
        registerConfirmLabel->setObjectName("registerConfirmLabel");

        registerFormLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, registerConfirmLabel);

        registerConfirmEdit = new QLineEdit(registerPage);
        registerConfirmEdit->setObjectName("registerConfirmEdit");
        registerConfirmEdit->setEchoMode(QLineEdit::EchoMode::Password);

        registerFormLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, registerConfirmEdit);

        registerCommentLabel = new QLabel(registerPage);
        registerCommentLabel->setObjectName("registerCommentLabel");

        registerFormLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, registerCommentLabel);

        registerCommentEdit = new QLineEdit(registerPage);
        registerCommentEdit->setObjectName("registerCommentEdit");

        registerFormLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, registerCommentEdit);


        registerLayout->addLayout(registerFormLayout);

        registerMessageLabel = new QLabel(registerPage);
        registerMessageLabel->setObjectName("registerMessageLabel");
        registerMessageLabel->setWordWrap(true);

        registerLayout->addWidget(registerMessageLabel);

        registerBtn = new QPushButton(registerPage);
        registerBtn->setObjectName("registerBtn");

        registerLayout->addWidget(registerBtn);

        registerVerticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        registerLayout->addItem(registerVerticalSpacer);

        tabs->addTab(registerPage, QString());

        rootLayout->addWidget(tabs);


        retranslateUi(LoginDialog);

        tabs->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(LoginDialog);
    } // setupUi

    void retranslateUi(QDialog *LoginDialog)
    {
        LoginDialog->setWindowTitle(QCoreApplication::translate("LoginDialog", "\354\230\244\353\252\251 \352\262\214\354\236\204 \353\241\234\352\267\270\354\235\270", nullptr));
        titleLabel->setText(QCoreApplication::translate("LoginDialog", "\354\230\244\353\252\251 \352\262\214\354\236\204", nullptr));
        loginNickLabel->setText(QCoreApplication::translate("LoginDialog", "\353\213\211\353\204\244\354\236\204 :", nullptr));
        loginNickEdit->setPlaceholderText(QCoreApplication::translate("LoginDialog", "\353\213\211\353\204\244\354\236\204", nullptr));
        loginPasswordLabel->setText(QCoreApplication::translate("LoginDialog", "\353\271\204\353\260\200\353\262\210\355\230\270 :", nullptr));
        loginPasswordEdit->setPlaceholderText(QCoreApplication::translate("LoginDialog", "\353\271\204\353\260\200\353\262\210\355\230\270", nullptr));
        loginMessageLabel->setText(QString());
        loginBtn->setText(QCoreApplication::translate("LoginDialog", "\353\241\234\352\267\270\354\235\270", nullptr));
        tabs->setTabText(tabs->indexOf(loginPage), QCoreApplication::translate("LoginDialog", "\353\241\234\352\267\270\354\235\270", nullptr));
        registerNickLabel->setText(QCoreApplication::translate("LoginDialog", "\353\213\211\353\204\244\354\236\204 :", nullptr));
        registerNickEdit->setPlaceholderText(QCoreApplication::translate("LoginDialog", "\352\262\214\354\236\204\354\227\220\354\204\234 \354\202\254\354\232\251\355\225\240 \353\213\211\353\204\244\354\236\204", nullptr));
        registerPasswordLabel->setText(QCoreApplication::translate("LoginDialog", "\353\271\204\353\260\200\353\262\210\355\230\270 :", nullptr));
        registerPasswordEdit->setPlaceholderText(QCoreApplication::translate("LoginDialog", "4\354\236\220 \354\235\264\354\203\201", nullptr));
        registerConfirmLabel->setText(QCoreApplication::translate("LoginDialog", "\355\231\225\354\235\270 :", nullptr));
        registerConfirmEdit->setPlaceholderText(QCoreApplication::translate("LoginDialog", "\353\271\204\353\260\200\353\262\210\355\230\270 \353\213\244\354\213\234 \354\236\205\353\240\245", nullptr));
        registerCommentLabel->setText(QCoreApplication::translate("LoginDialog", "\355\225\234\353\247\210\353\224\224 :", nullptr));
        registerCommentEdit->setPlaceholderText(QCoreApplication::translate("LoginDialog", "\355\225\234\353\247\210\353\224\224 (\354\204\240\355\203\235)", nullptr));
        registerMessageLabel->setText(QString());
        registerBtn->setText(QCoreApplication::translate("LoginDialog", "\355\232\214\354\233\220\352\260\200\354\236\205", nullptr));
        tabs->setTabText(tabs->indexOf(registerPage), QCoreApplication::translate("LoginDialog", "\355\232\214\354\233\220\352\260\200\354\236\205", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LoginDialog: public Ui_LoginDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINDIALOG_H
