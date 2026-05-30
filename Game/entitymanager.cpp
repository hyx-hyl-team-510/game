#include "EntityManager.h"
#include "Entity.h"
#include <QGraphicsScene>
#include <cmath>

// ========== EntityManager 实现 ==========

EntityManager* EntityManager::s_instance = nullptr;

EntityManager* EntityManager::instance()
{
    if (!s_instance) {
        s_instance = new EntityManager();
    }
    return s_instance;
}

EntityManager::EntityManager(QObject* parent)
    : QObject(parent)
{
    m_spatialGrid.cellSize = m_spatialCellSize;
}

EntityManager::~EntityManager()
{
    clearAll(false);
}

// ========== 实体创建/销毁 ==========

void EntityManager::registerEntity(Entity* entity)
{
    if (!entity) return;

    uint64_t id = entity->getUniqueId();

    if (m_entityMap.contains(id)) {
        qDebug() << "EntityManager: Entity already registered, id:" << id;
        return;
    }

    m_entityMap[id] = entity;
    m_typeMap[entity->getType()].append(entity);
    addEntityToTeamList(entity);

    connect(entity, &Entity::entityDestroyed, this, [this](Entity* e) {
        scheduleDestroy(e);
    });
    connect(entity, &Entity::teamChanged, this, [this](Entity* e, EntityTeam oldTeam, EntityTeam newTeam) {
        Q_UNUSED(oldTeam)
        removeEntityFromTeamList(e);
        addEntityToTeamList(e);
    });

    m_spatialGridDirty = true;

    emit entityAdded(entity);
    entity->onSpawn();
}

void EntityManager::destroyEntity(Entity* entity)
{
    if (!entity) return;
    scheduleDestroy(entity);
}

void EntityManager::destroyEntity(uint64_t id)
{
    destroyEntity(getEntityById(id));
}

void EntityManager::destroyAllEnemies()
{
    QVector<Entity*> enemies = m_teamEnemyList;
    for (Entity* e : enemies) {
        if (e->getType() == EntityType::ENEMY) {
            destroyEntity(e);
        }
    }
    processPendingOperations();
}

void EntityManager::destroyAllProjectiles()
{
    QVector<Entity*> projectiles = getEntitiesByType(EntityType::PROJECTILE);
    for (Entity* e : projectiles) {
        destroyEntity(e);
    }
    processPendingOperations();
}

void EntityManager::destroyAllItems()
{
    QVector<Entity*> items = getEntitiesByType(EntityType::ITEM);
    for (Entity* e : items) {
        destroyEntity(e);
    }
    processPendingOperations();
}

void EntityManager::destroyAllEffects()
{
    QVector<Entity*> effects = getEntitiesByType(EntityType::EFFECT);
    for (Entity* e : effects) {
        destroyEntity(e);
    }
    processPendingOperations();
}

void EntityManager::clearAll(bool keepPlayer)
{
    QVector<Entity*> toDestroy;
    for (auto it = m_entityMap.begin(); it != m_entityMap.end(); ++it) {
        Entity* e = it.value();
        if (keepPlayer && e == m_player) continue;
        toDestroy.append(e);
    }

    for (Entity* e : toDestroy) {
        if (e->scene() == m_scene) {
            m_scene->removeItem(e->graphicsItem());
        }
        e->onDestroy();
        delete e;
    }

    m_entityMap.clear();
    m_typeMap.clear();
    m_teamPlayerList.clear();
    m_teamEnemyList.clear();
    m_teamNeutralList.clear();
    m_destroyQueue.clear();
    m_addToSceneQueue.clear();
    m_removeFromSceneQueue.clear();
    m_spatialGrid.clear();
    m_spatialGridDirty = true;

    if (keepPlayer && m_player) {
        registerEntity(m_player);
    }
}

void EntityManager::addToScene(Entity* entity)
{
    if (!entity || !m_scene) return;
    if (entity->scene() == m_scene) return;
    scheduleAddToScene(entity);
}

void EntityManager::removeFromScene(Entity* entity)
{
    if (!entity || !m_scene) return;
    if (entity->scene() != m_scene) return;
    scheduleRemoveFromScene(entity);
}

// ========== 实体查询 ==========

Entity* EntityManager::getEntityById(uint64_t id) const
{
    return m_entityMap.value(id, nullptr);
}

QVector<Entity*> EntityManager::getEntitiesByType(EntityType type) const
{
    return m_typeMap.value(type, QVector<Entity*>());
}

QVector<Entity*> EntityManager::getEntitiesByTeam(EntityTeam team) const
{
    switch (team) {
    case EntityTeam::PLAYER: return m_teamPlayerList;
    case EntityTeam::ENEMY:  return m_teamEnemyList;
    default:                 return m_teamNeutralList;
    }
}

QVector<Entity*> EntityManager::getEntitiesInRadius(const QPointF& center, qreal radius,
                                                     EntityTeam team) const
{
    QVector<Entity*> result;
    qreal radiusSq = radius * radius;

    QVector<Entity*> candidates;
    if (m_spatialGridEnabled && !m_spatialGridDirty) {
        candidates = m_spatialGrid.queryRadius(center, radius);
    } else {
        candidates = getEntitiesByTeam(team);
    }

    for (Entity* e : candidates) {
        if (!e || !e->isActive() || e->getTeam() != team) continue;
        QPointF delta = e->pos() - center;
        qreal distSq = delta.x() * delta.x() + delta.y() * delta.y();
        if (distSq <= radiusSq) {
            result.append(e);
        }
    }

    return result;
}

QVector<Entity*> EntityManager::getEntitiesInRect(const QRectF& rect, EntityTeam team) const
{
    QVector<Entity*> result;

    QVector<Entity*> candidates;
    if (m_spatialGridEnabled && !m_spatialGridDirty) {
        candidates = m_spatialGrid.query(rect);
    } else {
        candidates = getEntitiesByTeam(team);
    }

    for (Entity* e : candidates) {
        if (!e || !e->isActive() || e->getTeam() != team) continue;
        if (rect.intersects(e->getCollisionRect())) {
            result.append(e);
        }
    }

    return result;
}

Entity* EntityManager::getNearestEntity(const QPointF& pos, EntityType type, qreal maxRange) const
{
    if (maxRange < 0) { return nullptr; }
    Entity* nearest = nullptr;
    qreal minDistSq = maxRange * maxRange;
    QVector<Entity*> candidates;
    if (m_spatialGridEnabled && !m_spatialGridDirty) {
        candidates = m_spatialGrid.queryRadius(pos,maxRange);
    }else{
        candidates = getEntitiesByType(type);
    }
    for (Entity* e : candidates) {
        if (!e || !e->isActive() || !e->isAlive() || e->getType() != type) continue;
        QPointF delta = e->pos() - pos;
        qreal distSq = delta.x() * delta.x() + delta.y() * delta.y();
        if (distSq < minDistSq) {
            minDistSq = distSq;
            nearest = e;
        }
    }

    return nearest;
}

Entity* EntityManager::getNearestEnemy(const QPointF& pos, qreal maxRange) const
{
    if (maxRange < 0) { return nullptr; }
    Entity* nearest = nullptr;
    qreal minDistSq = maxRange * maxRange;
    QVector<Entity*> candidates;
    if (m_spatialGridEnabled && !m_spatialGridDirty) {
        candidates = m_spatialGrid.queryRadius(pos,maxRange);
    }else{
        candidates = m_teamEnemyList;
    }
    for (Entity* e : candidates) {
        if (!e || !e->isActive() || !e->isAlive() || e->getType() != EntityType::ENEMY) continue;

        QPointF delta = e->pos() - pos;
        qreal distSq = delta.x() * delta.x() + delta.y() * delta.y();
        if (distSq < minDistSq) {
            minDistSq = distSq;
            nearest = e;
        }
    }

    return nearest;
}

Entity* EntityManager::getNearestEnemy(const Entity* cur, qreal maxRange) const{
    if (maxRange < 0) { return nullptr; }
    Entity* nearest = nullptr;
    qreal minDistSq = maxRange * maxRange;
    QVector<Entity*> candidates;
    if (m_spatialGridEnabled && !m_spatialGridDirty) {
        candidates = m_spatialGrid.queryRadius(cur->pos(),maxRange);
    }else{
        candidates = m_teamEnemyList;
    }
    for (Entity* e : candidates) {
        if (!e || !e->isActive() || !e->isAlive() || e->getType() != EntityType::ENEMY || e == cur) continue;

        QPointF delta = e->pos() - cur->pos();
        qreal distSq = delta.x() * delta.x() + delta.y() * delta.y();
        if (distSq < minDistSq) {
            minDistSq = distSq;
            nearest = e;
        }
    }

    return nearest;
}

Entity* EntityManager::getNearestAlly(const QPointF& pos, qreal maxRange) const
{
    if (maxRange < 0) { return nullptr; }
    Entity* nearest = nullptr;
    qreal minDistSq = maxRange * maxRange;

    for (Entity* e : m_teamPlayerList) {
        if (!e || !e->isActive() || !e->isAlive() || e == m_player) continue;

        QPointF delta = e->pos() - pos;
        qreal distSq = delta.x() * delta.x() + delta.y() * delta.y();
        if (distSq < minDistSq) {
            minDistSq = distSq;
            nearest = e;
        }
    }

    return nearest;
}

void EntityManager::setPlayer(Entity* player)
{
    if (m_player) {
        removeEntityFromTeamList(m_player);
    }
    m_player = player;
    if (m_player) {
        m_player->setTeam(EntityTeam::PLAYER);
        addEntityToTeamList(m_player);
    }
    emit playerSet(m_player);
}

Entity* EntityManager::getPet() const
{
    auto pets = getEntitiesByType(EntityType::PET);
    for (Entity* e : pets) {
        if (e->isActive() && e->isAlive()) {
            return e;
        }
    }
    return nullptr;
}

bool EntityManager::hasPet() const
{
    return getPet() != nullptr;
}

// ========== 统计 ==========

int EntityManager::getEntityCount(EntityType type) const
{
    return m_typeMap.value(type, QVector<Entity*>()).size();
}

int EntityManager::getEnemyCount() const
{
    return m_teamEnemyList.size();
}

bool EntityManager::hasAliveEnemies() const
{
    for (Entity* e : m_teamEnemyList) {
        if (e->isAlive()) return true;
    }
    return false;
}

// ========== 更新 ==========

void EntityManager::updateAll(float deltaTime)
{
    QVector<Entity*> entities = m_entityMap.values().toVector();
    for (Entity* e : entities) {
        if (e && e->isActive()) {
            e->update(deltaTime);
        }
    }
    m_spatialGridDirty = true;
}

void EntityManager::processPendingOperations()
{
    // 添加到场景
    for (Entity* e : m_addToSceneQueue) {
        if (e && m_scene && e->scene() != m_scene) {
            m_scene->addItem(e->graphicsItem());
        }
    }
    m_addToSceneQueue.clear();

    // 从场景移除
    for (Entity* e : m_removeFromSceneQueue) {
        if (e && m_scene && e->scene() == m_scene) {
            m_scene->removeItem(e->graphicsItem());
        }
    }
    m_removeFromSceneQueue.clear();

    // 销毁
    for (Entity* e : m_destroyQueue) {
        if (!e) continue;

        if (m_scene && e->scene() == m_scene) {
            m_scene->removeItem(e->graphicsItem());
        }

        removeEntityFromMaps(e);
        removeEntityFromTeamList(e);

        if (e == m_player) {
            m_player = nullptr;
        }

        e->onDestroy();
        emit entityRemoved(e);
        delete e;
    }
    m_destroyQueue.clear();
}

// ========== 空间划分 ==========

void EntityManager::updateSpatialGrid()
{
    if (!m_spatialGridEnabled) return;

    m_spatialGrid.clear();
    m_spatialGrid.cellSize = m_spatialCellSize;

    for (auto it = m_entityMap.begin(); it != m_entityMap.end(); ++it) {
        Entity* e = it.value();
        if (e && e->isActive()) {
            m_spatialGrid.insert(e);
        }
    }

    m_spatialGridDirty = false;
}

QVector<QPair<Entity*, Entity*>> EntityManager::getPotentialCollisions()
{
    QVector<QPair<Entity*, Entity*>> result;

    if (m_spatialGridEnabled) {
        if (m_spatialGridDirty) {
            updateSpatialGrid();
        }
        for (auto cellIt = m_spatialGrid.cells.begin(); cellIt != m_spatialGrid.cells.end(); ++cellIt) {
            QVector<Entity*>& cell = cellIt.value();
            int size = cell.size();

            for (int i = 0; i < size; ++i) {
                for (int j = i + 1; j < size; ++j) {
                    Entity* a = cell[i];
                    Entity* b = cell[j];
                    if (a->canCollideWith(b) && b->canCollideWith(a)) {
                        result.append(QPair<Entity*, Entity*>(a, b));
                    }
                }
            }
        }
    } else {
        QList<Entity*> entities = m_entityMap.values();
        int size = entities.size();
        for (int i = 0; i < size; ++i) {
            for (int j = i + 1; j < size; ++j) {
                Entity* a = entities[i];
                Entity* b = entities[j];
                if (a->canCollideWith(b) && b->canCollideWith(a)) {
                    result.append(QPair<Entity*, Entity*>(a, b));
                }
            }
        }
    }

    return result;
}

// ========== 调试 ==========

void EntityManager::dumpEntityStats() const
{
    qDebug() << "=== EntityManager Stats ===";
    qDebug() << "  Total entities:" << m_entityMap.size();
    qDebug() << "  Player:" << (m_player ? "exists" : "null");
    qDebug() << "  Pet:" << (hasPet() ? "exists" : "null");
    qDebug() << "  By type:";
    for (auto it = m_typeMap.begin(); it != m_typeMap.end(); ++it) {
        qDebug() << "   " << static_cast<int>(it.key()) << ":" << it.value().size();
    }
    qDebug() << "  By team:";
    qDebug() << "    PLAYER:" << m_teamPlayerList.size();
    qDebug() << "    ENEMY:" << m_teamEnemyList.size();
    qDebug() << "    NEUTRAL:" << m_teamNeutralList.size();
    qDebug() << "  Pending: add=" << m_addToSceneQueue.size()
             << " remove=" << m_removeFromSceneQueue.size()
             << " destroy=" << m_destroyQueue.size();
}

// ========== 私有方法 ==========

void EntityManager::removeEntityFromMaps(Entity* entity)
{
    if (!entity) return;

    uint64_t id = entity->getUniqueId();
    m_entityMap.remove(id);

    EntityType type = entity->getType();
    if (m_typeMap.contains(type)) {
        m_typeMap[type].removeOne(entity);
    }
}

void EntityManager::addEntityToTeamList(Entity* entity)
{
    if (!entity || entity->getType() == EntityType::PROJECTILE) return;

    switch (entity->getTeam()) {
    case EntityTeam::PLAYER:
        if (!m_teamPlayerList.contains(entity)) {
            m_teamPlayerList.append(entity);
        }
        break;
    case EntityTeam::ENEMY:
        if (!m_teamEnemyList.contains(entity)) {
            m_teamEnemyList.append(entity);
        }
        break;
    default:
        if (!m_teamNeutralList.contains(entity)) {
            m_teamNeutralList.append(entity);
        }
        break;
    }
}

void EntityManager::removeEntityFromTeamList(Entity* entity)
{
    if (!entity) return;

    switch (entity->getTeam()) {
    case EntityTeam::PLAYER:
        m_teamPlayerList.removeOne(entity);
        break;
    case EntityTeam::ENEMY:
        m_teamEnemyList.removeOne(entity);
        break;
    default:
        m_teamNeutralList.removeOne(entity);
        break;
    }
}

void EntityManager::scheduleDestroy(Entity* entity)
{
    if (entity && !m_destroyQueue.contains(entity)) {
        m_destroyQueue.append(entity);
    }
}

void EntityManager::scheduleAddToScene(Entity* entity)
{
    if (entity && !m_addToSceneQueue.contains(entity)) {
        m_addToSceneQueue.append(entity);
    }
}

void EntityManager::scheduleRemoveFromScene(Entity* entity)
{
    if (entity && !m_removeFromSceneQueue.contains(entity)) {
        m_removeFromSceneQueue.append(entity);
    }
}

// ========== SpatialGrid 实现 ==========

void EntityManager::SpatialGrid::clear()
{
    cells.clear();
}

void EntityManager::SpatialGrid::insert(Entity* entity)
{
    if (!entity) return;

    QPair<int, int> cell = getCell(entity->pos());
    cells[cell].append(entity);
}

QVector<Entity*> EntityManager::SpatialGrid::query(const QRectF& rect) const
{
    QVector<Entity*> result;

    int minX = static_cast<int>(std::floor(rect.left() / cellSize));
    int maxX = static_cast<int>(std::floor(rect.right() / cellSize));
    int minY = static_cast<int>(std::floor(rect.top() / cellSize));
    int maxY = static_cast<int>(std::floor(rect.bottom() / cellSize));

    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            QPair<int, int> cell(x, y);
            if (cells.contains(cell)) {
                result.append(cells[cell]);
            }
        }
    }

    return result;
}

QVector<Entity*> EntityManager::SpatialGrid::queryRadius(const QPointF& center, qreal radius) const
{
    QRectF rect(center.x() - radius, center.y() - radius, radius * 2, radius * 2);
    return query(rect);
}

QPair<int, int> EntityManager::SpatialGrid::getCell(const QPointF& pos) const
{
    int x = static_cast<int>(std::floor(pos.x() / cellSize));
    int y = static_cast<int>(std::floor(pos.y() / cellSize));
    return {x, y};
}
