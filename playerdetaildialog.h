#pragma once
#include <QDialog>
#include "database.h"

class PlayerDetailDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PlayerDetailDialog(const Player& player, QWidget* parent = nullptr);
};
