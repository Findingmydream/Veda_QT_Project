#include "playersearchdialog.h"

#include <QVBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QLabel>

PlayerSearchDialog::PlayerSearchDialog(const QStringList& names, QWidget* parent)
    : QDialog(parent)
    , allNames_(names)
{
    setWindowTitle("상대 검색");
    setModal(true);
    setFixedSize(320, 400);

    auto* lay = new QVBoxLayout(this);

    auto* hint = new QLabel("닉네임을 입력하면 일치하는 상대가 표시됩니다.\n원하는 이름을 클릭하세요.");
    hint->setStyleSheet("color:#888;");
    hint->setWordWrap(true);
    lay->addWidget(hint);

    searchEdit_ = new QLineEdit;
    searchEdit_->setPlaceholderText("닉네임...");
    lay->addWidget(searchEdit_);

    list_ = new QListWidget;
    lay->addWidget(list_, 1);

    onTextChanged(QString());

    connect(searchEdit_, &QLineEdit::textChanged, this, &PlayerSearchDialog::onTextChanged);
    connect(list_,       &QListWidget::itemClicked, this, &PlayerSearchDialog::onItemClicked);
}

void PlayerSearchDialog::onTextChanged(const QString& text)
{
    list_->clear();
    QString needle = text.trimmed();
    for (const QString& name : allNames_) {
        if (needle.isEmpty() || name.contains(needle, Qt::CaseInsensitive))
            list_->addItem(name);
    }
}

void PlayerSearchDialog::onItemClicked(QListWidgetItem* item)
{
    if (!item) return;
    selected_ = item->text();
    accept();
}
