#ifndef GAMEAPPLICATION_H
#define GAMEAPPLICATION_H

#include <QObject>
#include <QScopedPointer>

class GameWindow;

class GameApplication : public QObject
{
    Q_OBJECT

public:
    explicit GameApplication(QObject *parent = nullptr);
    ~GameApplication();

    void initialize();
    void shutdown();

    // 全局系统访问接口
    class CombatSystem* getCombatSystem() const;
    class InventorySystem* getInventorySystem() const;
    class UpgradeSystem* getUpgradeSystem() const;

private:
    QScopedPointer<GameWindow> m_gameWindow;

    // 各系统实例
    QScopedPointer<class CombatSystem> m_combatSystem;
    QScopedPointer<class InventorySystem> m_inventorySystem;
    QScopedPointer<class UpgradeSystem> m_upgradeSystem;
};

#endif // GAMEAPPLICATION_H
