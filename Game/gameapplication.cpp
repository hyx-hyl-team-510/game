#include "GameApplication.h"
#include "GameWindow.h"
#include "CombatSystem.h"
#include "InventorySystem.h"
#include "UpgradeSystem.h"

GameApplication::GameApplication(QObject *parent)
    : QObject(parent)
{
}

GameApplication::~GameApplication()
{
    shutdown();
}

void GameApplication::initialize()
{
    // 初始化各系统
    m_combatSystem.reset(new CombatSystem(this));
    m_inventorySystem.reset(new InventorySystem(this));
    m_upgradeSystem.reset(new UpgradeSystem(this));

    // 初始化主窗口
    m_gameWindow.reset(new GameWindow(this));
    m_gameWindow->show();
}

void GameApplication::shutdown()
{
    if (m_gameWindow) {
        m_gameWindow->close();
    }
}

CombatSystem* GameApplication::getCombatSystem() const
{
    return m_combatSystem.data();
}

InventorySystem* GameApplication::getInventorySystem() const
{
    return m_inventorySystem.data();
}


UpgradeSystem* GameApplication::getUpgradeSystem() const
{
    return m_upgradeSystem.data();
}
