#include "InventorySystem.h"
#include <QDebug>

InventorySystem::InventorySystem(QObject* parent)
    : QObject(parent)
    , m_capacity(20)
    , m_gold(0)
{
}

InventorySystem::~InventorySystem()
{
}

void InventorySystem::addItem(int itemId, int count)
{
    if (m_items.contains(itemId)) {
        m_items[itemId] += count;
    } else {
        m_items[itemId] = count;
    }
    emit itemAdded(itemId, count);
    qDebug() << "Added item" << itemId << "x" << count;
}

void InventorySystem::removeItem(int itemId, int count)
{
    if (!m_items.contains(itemId)) return;

    m_items[itemId] -= count;
    if (m_items[itemId] <= 0) {
        m_items.remove(itemId);
    }
    emit itemRemoved(itemId, count);
}

int InventorySystem::getItemCount(int itemId) const
{
    return m_items.value(itemId, 0);
}

bool InventorySystem::equipItem(int itemId, int slot)
{
    if (!m_items.contains(itemId) || m_items[itemId] <= 0) {
        return false;
    }

    m_equipped[slot] = itemId;
    emit itemEquipped(itemId, slot);
    return true;
}

bool InventorySystem::unequipItem(int slot)
{
    if (!m_equipped.contains(slot)) return false;

    m_equipped.remove(slot);
    return true;
}

int InventorySystem::getEquippedItem(int slot) const
{
    return m_equipped.value(slot, -1);
}

void InventorySystem::addGold(int amount)
{
    m_gold += amount;
    emit goldChanged(m_gold);
}

bool InventorySystem::spendGold(int amount)
{
    if (m_gold < amount) return false;

    m_gold -= amount;
    emit goldChanged(m_gold);
    return true;
}
