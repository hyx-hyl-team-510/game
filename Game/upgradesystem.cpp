#include "UpgradeSystem.h"
#include "Player.h"
#include <QDebug>

UpgradeSystem::UpgradeSystem(QObject* parent)
    : QObject(parent)
{
    initializeCosts();
}

UpgradeSystem::~UpgradeSystem()
{
}

void UpgradeSystem::initializeCosts()
{
    m_upgradeCosts[UPGRADE_HEALTH] = 100;
    m_upgradeCosts[UPGRADE_MANA] = 100;
    m_upgradeCosts[UPGRADE_ATTACK] = 150;
    m_upgradeCosts[UPGRADE_DEFENSE] = 150;
    m_upgradeCosts[UPGRADE_SPEED] = 200;
    m_upgradeCosts[UPGRADE_CRIT_CHANCE] = 250;
    m_upgradeCosts[UPGRADE_CRIT_DAMAGE] = 300;

    for (int i = 0; i < 7; i++) {
        m_upgradeLevels[static_cast<UpgradeType>(i)] = 0;
    }
}


int UpgradeSystem::getUpgradeLevel(UpgradeType type) const
{
    return m_upgradeLevels.value(type, 0);
}

int UpgradeSystem::getUpgradeCost(UpgradeType type) const
{
    return m_upgradeCosts.value(type, 100);
}

void UpgradeSystem::resetUpgrades()
{
    for (int i = 0; i < 7; i++) {
        m_upgradeLevels[static_cast<UpgradeType>(i)] = 0;
    }
    initializeCosts();
}
