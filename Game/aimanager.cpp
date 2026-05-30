#include "aimanager.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

// 静态成员初始化
AIManager* AIManager::m_instance = nullptr;

AIManager* AIManager::instance()
{
    if (!m_instance) {
        m_instance = new AIManager();
    }
    return m_instance;
}

AIManager::AIManager(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
}

AIManager::~AIManager()
{
    m_instance = nullptr;
}

void AIManager::requestWeapon(const QString& context,
                                      std::function<void(const QString&, int, const QString&)> onSuccess,
                                      std::function<void(const QString&)> onError)
{
    QUrl url("https://api.deepseek.com/v1/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization",QString("Bearer %1").arg(m_key).toUtf8());

    // 构造请求体
    QJsonObject json;
    json["model"] = "deepseek-chat";
    json["temperature"] = 1.2;

    QJsonArray messages;

    QJsonObject systemMsg;
    systemMsg["role"] = "system";
    systemMsg["content"] = R"(
        你是一个游戏武器生成器。

        【重要】type 字段必须随机选择，50%概率是"剑"，50%概率是"弓"。
        - 不要总选同一种
        - 不要输出其他type值

        只输出JSON，格式：
        {"name": "武器名", "attack": 攻击力(20-80), "type": "剑"}

        或
        {"name": "武器名", "attack": 攻击力(20-80), "type": "弓"}
    )";
    messages.append(systemMsg);

    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = context;
    messages.append(userMsg);

    json["messages"] = messages;

    QNetworkReply* reply = m_manager->post(request, QJsonDocument(json).toJson());

    // 保存回调
    m_successCallbacks[reply] = onSuccess;
    m_errorCallbacks[reply] = onError;

    connect(reply, &QNetworkReply::finished, this, &AIManager::onReplyFinished);
}

void AIManager::requestBossTalk(const QString& context,
                                std::function<void(const QString&)> onSuccess,
                                std::function<void(const QString&)> onError)
{
    QUrl url("https://api.deepseek.com/v1/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_key).toUtf8());

    // 构造请求体
    QJsonObject json;
    json["model"] = "deepseek-chat";
    json["temperature"] = 1.0;  // BOSS 台词可以稍微保守一点

    QJsonArray messages;

    QJsonObject systemMsg;
    systemMsg["role"] = "system";
    systemMsg["content"] = R"(
    你是一个游戏中的BOSS角色。根据玩家的状态,说一句挑衅或霸气的话。

    【严格要求】
    - 字数必须在20字以内（含标点符号）
    - 只输出纯文本，不要输出JSON，不要加引号
    - 不要输出任何解释或额外内容
    - 要符合BOSS的身份，要有压迫感或嘲讽感

    示例：
    "蝼蚁，也敢挑战我？"
    "你的旅途到此为止了。"
    "就这点本事？"
    )";
    messages.append(systemMsg);

    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = context;
    messages.append(userMsg);

    json["messages"] = messages;

    QNetworkReply* reply = m_manager->post(request, QJsonDocument(json).toJson());

    // 保存回调
    m_bossTalkCallbacks[reply] = onSuccess;
    m_errorCallbacks[reply] = onError;

    connect(reply, &QNetworkReply::finished, this, &AIManager::onBossTalkReplyFinished);
}

void AIManager::onReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    // 取出回调
    auto successIt = m_successCallbacks.find(reply);
    auto errorIt = m_errorCallbacks.find(reply);

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "API Error:" << reply->errorString();
        if (errorIt != m_errorCallbacks.end() && errorIt.value()) {
            errorIt.value()(reply->errorString());
        }
    } else {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();

        QString content = obj["choices"][0]["message"]["content"].toString();
        QJsonDocument weaponDoc = QJsonDocument::fromJson(content.toUtf8());
        QJsonObject weaponObj = weaponDoc.object();

        QString name = weaponObj["name"].toString("AI武器");
        int attack = weaponObj["attack"].toInt(15);
        QString type = weaponObj["type"].toString("近战");

        if (successIt != m_successCallbacks.end() && successIt.value()) {
            successIt.value()(name, attack, type);
        }
    }

    // 清理
    m_successCallbacks.remove(reply);
    m_errorCallbacks.remove(reply);
    reply->deleteLater();
}

void AIManager::onBossTalkReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    auto successIt = m_bossTalkCallbacks.find(reply);
    auto errorIt = m_errorCallbacks.find(reply);

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "BOSS Talk API Error:" << reply->errorString();
        if (errorIt != m_errorCallbacks.end() && errorIt.value()) {
            errorIt.value()(reply->errorString());
        }
    } else {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();

        // 提取 AI 回复的文本
        QString talk = obj["choices"][0]["message"]["content"].toString();

        // 清理：去掉首尾空格、换行、多余的引号
        talk = talk.trimmed();
        if (talk.startsWith('"') && talk.endsWith('"')) {
            talk = talk.mid(1, talk.length() - 2);
        }

        // 强制限制20字（如果超过，截断并加...）
        if (talk.length() > 20) {
            talk = talk.left(17) + "...";
            qWarning() << "BOSS台词超过20字，已截断:" << talk;
        }

        if (successIt != m_bossTalkCallbacks.end() && successIt.value()) {
            successIt.value()(talk);
        }
    }

    // 清理
    m_bossTalkCallbacks.remove(reply);
    m_errorCallbacks.remove(reply);
    reply->deleteLater();
}
