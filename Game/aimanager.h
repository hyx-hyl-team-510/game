#ifndef AIMANAGER_H
#define AIMANAGER_H
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <functional>
#include <QString>

class AIManager : public QObject
{
    Q_OBJECT

public:
    // 禁止拷贝和赋值
    AIManager(const AIManager&) = delete;
    AIManager& operator=(const AIManager&) = delete;

    // 全局访问点
    static AIManager* instance();

    // 请求武器生成
    void requestWeapon(const QString& context,
                       std::function<void(const QString& name, int attack, const QString& type)> onSuccess,
                       std::function<void(const QString& error)> onError = nullptr);
    // 请求 BOSS 台词生成
    void requestBossTalk(const QString& context,
                         std::function<void(const QString& talk)> onSuccess,
                         std::function<void(const QString& error)> onError = nullptr);
    void setEnabled(bool enabled) {m_enabled = enabled;}
    bool isEnabled() {return m_enabled;}
    void setKey(QString key) {m_key = key;}
private slots:
    void onReplyFinished();
    void onBossTalkReplyFinished();

private:
    explicit AIManager(QObject *parent = nullptr);
    ~AIManager();

    static AIManager* m_instance;
    QNetworkAccessManager* m_manager;

    // 存储当前请求的回调（用 reply 做 key）
    QMap<QNetworkReply*, std::function<void(const QString&, int, const QString&)>> m_successCallbacks;
    QMap<QNetworkReply*, std::function<void(const QString&)>> m_errorCallbacks;
    QMap<QNetworkReply*, std::function<void(const QString&)>> m_bossTalkCallbacks;
    QString m_key;
    bool m_enabled = false;
};
#endif // AIMANAGER_H
