#include "geminiclient.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QDebug>

// 주의: 이 키는 공개 대화에 노출됐었으므로 실제 서비스 전에는 반드시 재발급.
// 장기적으로는 QSettings 에 저장하거나 환경변수에서 읽는 게 안전함.
static const char* kGeminiApiKey = "AIzaSyDluADMwUfvZDHG57piJR4rzmuoPw3WzDo";
// 모델별 프로젝트 무료 할당량이 다를 수 있어, 최신 라인으로 시도.
// 여기서 또 429(limit: 0) 나오면 "gemini-2.5-flash" / "gemini-1.5-flash-8b" 순으로 시도.
static const char* kGeminiModel  = "gemini-2.5-flash-lite";

GeminiClient::GeminiClient(QObject* parent)
    : QObject(parent)
    , nam_(new QNetworkAccessManager(this))
{
}

void GeminiClient::requestText(const QString& prompt)
{
    QJsonObject part;
    part["text"] = prompt;
    QJsonArray parts; parts.append(part);
    QJsonObject content;
    content["parts"] = parts;
    QJsonArray contents; contents.append(content);

    QJsonObject generationConfig;
    generationConfig["temperature"]     = 0.4;
    generationConfig["maxOutputTokens"] = 50;

    QJsonObject body;
    body["contents"]         = contents;
    body["generationConfig"] = generationConfig;

    QUrl url(QString("https://generativelanguage.googleapis.com/v1beta/models/%1:generateContent?key=%2")
             .arg(kGeminiModel, kGeminiApiKey));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = nam_->post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, &GeminiClient::onReplyFinished);
}

void GeminiClient::onReplyFinished()
{
    auto* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        QByteArray body = reply->readAll();
        qDebug().noquote() << "[Gemini] error body:" << body;
        emit failed(QString("network: %1").arg(reply->errorString()));
        return;
    }

    auto doc = QJsonDocument::fromJson(reply->readAll());
    if (!doc.isObject()) { emit failed("invalid json"); return; }

    auto cands = doc.object().value("candidates").toArray();
    if (cands.isEmpty()) { emit failed("no candidates"); return; }

    auto partsArr = cands.first().toObject()
                        .value("content").toObject()
                        .value("parts").toArray();
    if (partsArr.isEmpty()) { emit failed("no parts"); return; }

    QString text = partsArr.first().toObject().value("text").toString();
    emit textReady(text);
}
