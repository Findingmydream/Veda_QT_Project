#pragma once
#include <QDialog>
#include <QStringList>

class QLineEdit;
class QListWidget;
class QListWidgetItem;

class PlayerSearchDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PlayerSearchDialog(const QStringList& names, QWidget* parent = nullptr);

    QString selectedName() const { return selected_; }

private slots:
    void onTextChanged(const QString& text);
    void onItemClicked(QListWidgetItem* item);

private:
    QLineEdit*   searchEdit_ = nullptr;
    QListWidget* list_       = nullptr;
    QStringList  allNames_;
    QString      selected_;
};
