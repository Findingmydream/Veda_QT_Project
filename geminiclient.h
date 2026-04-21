#pragma once
#include <QObject>
#include <QString>

class QNetworkAccessManager;

class GeminiClient : public QObject
{
    Q_OBJECT
public:
    explicit GeminiClient(QObject* parent = nullptr);

    // prompt 를 Gemini API 에 POST. 성공 시 textReady, 실패 시 failed 시그널.
    void requestText(const QString& prompt);

signals:
    void textReady(const QString& text);
    void failed(const QString& reason);

private slots:
    void onReplyFinished();

private:
    QNetworkAccessManager* nam_ = nullptr;
};
