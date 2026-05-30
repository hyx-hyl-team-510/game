#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include <QObject>
#include <QHash>
#include <QVector>
#include <QPair>
#include <QPointF>
#include <QRectF>
#include <QDebug>
#include <QGraphicsScene>
#include "entity.h"
class Entity;

class EntityManager : public QObject {
    Q_OBJECT

public:
    static EntityManager* instance();

    // 设置场景
    void setScene(QGraphicsScene* scene) { m_scene = scene; }
    QGraphicsScene* getScene() const { return m_scene; }

    // ========== 实体创建/销毁 ==========

    template<typename T, typename... Args>
    T* createEntity(Args&&... args) {
        T* entity = new T(std::forward<Args>(args)...);
        registerEntity(entity);
        return entity;
    }

    void registerEntity(Entity* entity);
    void destroyEntity(Entity* entity);
    void destroyEntity(uint64_t id);

    // 批量销毁
    void destroyAllEnemies();
    void destroyAllProjectiles();
    void destroyAllItems();
    void destroyAllEffects();
    void clearAll(bool keepPlayer = true);

    // 场景管理
    void addToScene(Entity* entity);
    void removeFromScene(Entity* entity);

    // ========== 实体查询 ==========

    Entity* getEntityById(uint64_t id) const;

    template<typename T>
    T* getEntityById(uint64_t id) const {
        return dynamic_cast<T*>(getEntityById(id));
    }

    QVector<Entity*> getEntitiesByType(EntityType type) const;

    template<typename T>
    QVector<T*> getEntitiesByType() const {
        QVector<T*> result;
        EntityType type = T::staticEntityType();
        auto it = m_typeMap.find(type);
        if (it != m_typeMap.end()) {
            for (Entity* e : it.value()) {
                if (T* casted = dynamic_cast<T*>(e)) {
                    result.append(casted);
                }
            }
        }
        return result;
    }

    QVector<Entity*> getEntitiesByTeam(EntityTeam team) const;
    QVector<Entity*> getEntitiesInRadius(const QPointF& center, qreal radius,
                                          EntityTeam team = EntityTeam::NEUTRAL) const;
    QVector<Entity*> getEntitiesInRect(const QRectF& rect,
                                        EntityTeam team = EntityTeam::NEUTRAL) const;

    Entity* getNearestEntity(const QPointF& pos, EntityType type, qreal maxRange) const;
    Entity* getNearestEnemy(const QPointF& pos, qreal maxRange) const;//玩家版本
    Entity* getNearestEnemy(const Entity* cur, qreal maxRange) const;//敌人版本
    Entity* getNearestAlly(const QPointF& pos, qreal maxRange) const;

    // 玩家管理
    Entity* getPlayer() const { return m_player; }
    void setPlayer(Entity* player);
    //商店管理
    Entity* getShopItem() const {return m_shopItem;}
    void setShopItem(Entity* shopItem){registerEntity(shopItem);m_shopItem = shopItem;}
    // 宠物快捷访问
    Entity* getPet() const;
    bool hasPet() const;

    // ========== 统计 ==========

    int getEntityCount(EntityType type) const;
    int getTotalEntityCount() const { return m_entityMap.size(); }
    int getEnemyCount() const;
    bool hasAliveEnemies() const;

    // ========== 更新 ==========

    void updateAll(float deltaTime);
    void processPendingOperations();

    // ========== 空间划分 ==========

    void updateSpatialGrid();
    QVector<QPair<Entity*, Entity*>> getPotentialCollisions();
    void setSpatialGridEnabled(bool enabled) { m_spatialGridEnabled = enabled; }
    void setSpatialCellSize(int size) { m_spatialCellSize = size; }

    // ========== 调试 ==========

    void dumpEntityStats() const;

signals:
    void entityAdded(Entity* entity);
    void entityRemoved(Entity* entity);
    void playerSet(Entity* player);

private:
    EntityManager(QObject* parent = nullptr);
    ~EntityManager();

    static EntityManager* s_instance;

    QGraphicsScene* m_scene = nullptr;

    // 实体存储
    QHash<uint64_t, Entity*> m_entityMap;
    QHash<EntityType, QVector<Entity*>> m_typeMap;
    QVector<Entity*> m_teamPlayerList;
    QVector<Entity*> m_teamEnemyList;
    QVector<Entity*> m_teamNeutralList;

    Entity* m_player = nullptr;
    Entity* m_shopItem = nullptr;

    // 待处理操作
    QVector<Entity*> m_destroyQueue;
    QVector<Entity*> m_addToSceneQueue;
    QVector<Entity*> m_removeFromSceneQueue;

    void removeEntityFromMaps(Entity* entity);
    void addEntityToTeamList(Entity* entity);
    void removeEntityFromTeamList(Entity* entity);
    void scheduleDestroy(Entity* entity);
    void scheduleAddToScene(Entity* entity);
    void scheduleRemoveFromScene(Entity* entity);

    // 空间划分
    struct SpatialGrid {
        int cellSize = 100;
        QHash<QPair<int, int>, QVector<Entity*>> cells;

        void clear();
        void insert(Entity* entity);
        QVector<Entity*> query(const QRectF& rect) const;
        QVector<Entity*> queryRadius(const QPointF& center, qreal radius) const;

    private:
        QPair<int, int> getCell(const QPointF& pos) const;
    };

    SpatialGrid m_spatialGrid;
    bool m_spatialGridEnabled = true;
    int m_spatialCellSize = 100;
    bool m_spatialGridDirty = true;
};

#endif // ENTITYMANAGER_H
