#pragma once
#include <QDialog>
#include "database.h"

class QTableWidget;
class QLabel;
class QPushButton;

class PlayerDetailDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PlayerDetailDialog(const Player& player, QWidget* parent = nullptr);

private slots:
    void onOpponentSearch();

private:
    void refreshOpponentTab(const QString& opponent);

    Player        player_;
    QTableWidget* personalTable_       = nullptr;

    QPushButton*  opponentSearchBtn_   = nullptr;
    QLabel*       opponentNameLabel_   = nullptr;
    QLabel*       opponentTotalLabel_  = nullptr;
    QLabel*       opponentWLDLabel_    = nullptr;
    QLabel*       opponentWinRateLabel_= nullptr;
    QTableWidget* opponentTable_       = nullptr;
};
