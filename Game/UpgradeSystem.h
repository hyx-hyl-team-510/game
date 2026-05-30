#ifndef UPGRADESYSTEM_H
#define UPGRADESYSTEM_H

#include <QObject>
#include <QMap>

class Player;

class UpgradeSystem : public QObject
{
    Q_OBJECT

public:
    explicit UpgradeSystem(QObject* parent = nullptr);
    ~UpgradeSystem();

    enum UpgradeType {
        UPGRADE_HEALTH,
        UPGRADE_MANA,
        UPGRADE_ATTACK,
        UPGRADE_DEFENSE,
        UPGRADE_SPEED,
        UPGRADE_CRIT_CHANCE,
        UPGRADE_CRIT_DAMAGE
    };

    int getUpgradeLevel(UpgradeType type) const;
    int getUpgradeCost(UpgradeType type) const;

    void resetUpgrades();

signals:
    void statUpgraded(UpgradeType type, int newLevel);

private:
    void initializeCosts();

    QMap<UpgradeType, int> m_upgradeLevels;
    QMap<UpgradeType, int> m_upgradeCosts;
};

#endif // UPGRADESYSTEM_H
