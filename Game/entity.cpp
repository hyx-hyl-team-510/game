#include "Entity.h"
#include <QGraphicsScene>
#include <QDebug>
#include <cmath>

// ========== Entity 实现 ==========

uint64_t Entity::s_nextId = 1;

Entity::Entity(EntityType type, QObject* parent)
    : QObject(parent)
    , m_type(type)
    , m_uniqueId(s_nextId++)
{
}

Entity::~Entity()
{
    emit entityDestroyed(this);
}

// ========== 便捷方法（转发到图形项） ==========

QPointF Entity::pos() const
{
    return graphicsItem()->pos();
}

void Entity::setPos(const QPointF& pos)
{
    graphicsItem()->setPos(pos);
}

void Entity::setPos(qreal x, qreal y)
{
    graphicsItem()->setPos(x, y);
}

qreal Entity::x() const
{
    return graphicsItem()->x();
}

qreal Entity::y() const
{
    return graphicsItem()->y();
}

QGraphicsScene* Entity::scene() const
{
    return graphicsItem()->scene();
}

bool Entity::isVisible() const
{
    return graphicsItem()->isVisible();
}

void Entity::setVisible(bool visible)
{
    graphicsItem()->setVisible(visible);
}

void Entity::update()
{
    graphicsItem()->update();
}

// ========== 核心属性 ==========

void Entity::setTeam(EntityTeam team)
{
    if (m_team == team) return;
    EntityTeam oldTeam = m_team;
    m_team = team;
    emit teamChanged(this, oldTeam, m_team);
}

void Entity::setActive(bool active)
{
    m_isActive = active;
    setVisible(active);
}

// ========== 碰撞 ==========

QRectF Entity::getCollisionRect() const
{
    return graphicsItem()->boundingRect().translated(pos());
}

bool Entity::canCollideWith(Entity* other) const
{
    if (!other) return false;
    if (!m_isActive || !other->m_isActive) return false;
    if (!isAlive() || !other->isAlive()) return false;

    // 同队伍不碰撞
    if (m_team != EntityTeam::NEUTRAL && m_team == other->m_team) {
        return false;
    }

    return true;
}

// ========== 属性 ==========

void Entity::setMaxHealth(int maxHealth)
{
    m_maxHealth = maxHealth;
    if (m_health > m_maxHealth) {
        m_health = m_maxHealth;
    }
}

void Entity::takeDamage(int damage, Entity* source)
{
    if (damage <= 0) return;
    if (!isAlive()) return;

    int oldHealth = m_health;
    m_health = qMax(0, m_health - damage);

    emit healthChanged(this, oldHealth, m_health);
    emit damageTaken(this, damage, source);
}

void Entity::heal(int amount)
{
    if (amount <= 0) return;
    if (!isAlive()) return;

    int oldHealth = m_health;
    m_health = qMin(m_maxHealth, m_health + amount);

    emit healthChanged(this, oldHealth, m_health);
    emit entityHealed(this, amount);
}

