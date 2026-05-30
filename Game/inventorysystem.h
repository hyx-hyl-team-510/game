#ifndef INVENTORYSYSTEM_H
#define INVENTORYSYSTEM_H

#include <QObject>
#include <QMap>
#include <QVector>

class Player;

class InventorySystem : public QObject
{
    Q_OBJECT

public:
    explicit InventorySystem(QObject* parent = nullptr);
    ~InventorySystem();

    // 物品管理
    void addItem(int itemId, int count);
    void removeItem(int itemId, int count);
    int getItemCount(int itemId) const;

    // 装备管理
    bool equipItem(int itemId, int slot);
    bool unequipItem(int slot);
    int getEquippedItem(int slot) const;

    // 背包容量
    int getCapacity() const { return m_capacity; }
    void setCapacity(int capacity) { m_capacity = capacity; }

    // 金币
    int getGold() const { return m_gold; }
    void addGold(int amount);
    bool spendGold(int amount);

signals:
    void itemAdded(int itemId, int count);
    void itemRemoved(int itemId, int count);
    void goldChanged(int newGold);
    void itemEquipped(int itemId, int slot);

private:
    QMap<int, int> m_items;
    QMap<int, int> m_equipped;
    int m_capacity;
    int m_gold;
};

#endif // INVENTORYSYSTEM_H
