#include "CombatSystem.h"
#include "Player.h"
#include "Enemy.h"
#include "Projectile.h"
#include <QRandomGenerator>
#include <QtMath>
#include <QDebug>
#include <QGraphicsScene>
#include "item.h"
// ========== 初始化静态参数 ==========
// 近战攻击参数
qreal CombatSystem::MELEE_RANGE = 40.0;
qreal CombatSystem::MELEE_DAMAGE_BASE = 20.0;
int CombatSystem::MELEE_COOLDOWN = 20;

// 远程攻击参数
qreal CombatSystem::RANGE_ATTACK_RANGE = 300.0;
qreal CombatSystem::PROJECTILE_SPEED = 8.0;
qreal CombatSystem::PROJECTILE_RADIUS = 15.0;
int CombatSystem::RANGE_COOLDOWN = 30;
qreal CombatSystem::RANGE_DAMAGE_BASE = 15.0;

CombatSystem::CombatSystem(QObject* parent)
    : QObject(parent)
{
    initializeSkills();
}

void CombatSystem::initializeSkills()
{
    SkillInfo fireball;
    fireball.id = 1;
    fireball.name = "Fireball";
    fireball.damage = 50;
    fireball.manaCost = 20;
    fireball.cooldown = 30;
    fireball.range = 300;
    fireball.areaRadius = 80;
    fireball.type = "area";
    m_skills[1] = fireball;

    SkillInfo heal;
    heal.id = 2;
    heal.name = "Heal";
    heal.damage = -30;
    heal.manaCost = 15;
    heal.cooldown = 60;
    heal.range = 0;
    heal.areaRadius = 0;
    heal.type = "support";
    m_skills[2] = heal;
}

QPointF CombatSystem::getDirectionVector(const QPointF& direction)
{
    qreal length = sqrt(direction.x() * direction.x() + direction.y() * direction.y());
    if (length > 0.01) {
        return QPointF(direction.x() / length, direction.y() / length);
    }
    return QPointF(1.0, 0.0);
}

QVector<Enemy*> CombatSystem::getEnemiesInCircularArea(const QPointF& center, qreal radius)
{
    QVector<Enemy*> result;
    // 这个函数需要访问场景中的敌人，将在 GameWindow 中实现
    // 这里返回空，实际使用时通过 GameWindow 传入敌人列表
    return result;
}

QVector<Player*> CombatSystem::getPlayersInCircularArea(const QPointF& center, qreal radius, Player* excludePlayer)
{
    Q_UNUSED(center);
    Q_UNUSED(radius);
    Q_UNUSED(excludePlayer);
    // 这个函数需要访问场景中的玩家，将在 GameWindow 中实现
    return QVector<Player*>();
}

// ========== 近战攻击实现 ==========
void CombatSystem::meleeAttack(Player* attacker, const QPointF& direction, QVector<Enemy*> target)
{
    if (!attacker || attacker->isDead()) return;
    QPointF dirVec = getDirectionVector(direction);
    QPointF center = attacker->pos() + dirVec * MELEE_RANGE;

    int totalDamage = calculateDamage(static_cast<int>(MELEE_DAMAGE_BASE),attacker->getTotalAttack());  // 使用总攻击力

    for (Enemy* enemy : target) {
        if (!enemy || enemy->isDead()) continue;

        QPointF delta = enemy->pos() - center;
        qreal dist = sqrt(delta.x() * delta.x() + delta.y() * delta.y());

        if (dist <= MELEE_RANGE) {
            enemy->knockback(enemy->pos() - attacker->pos(),2);
            enemy->takeDamage(totalDamage);
            emit playerDamageDealt(attacker, enemy, totalDamage);
            qDebug() << "Player melee hit enemy, damage:" << totalDamage;
        }
    }
}


void CombatSystem::meleeAttack(Enemy* attacker,Player *target)
{
    if (!attacker || attacker->isDead() || !target || target->isDead()) return;
    target->takeDamage(attacker->getDamage());
    emit enemyDamageDealt(attacker, target, attacker->getDamage());
    qDebug() << "Enemy melee hit player, damage:" << attacker->getDamage();
    return;
}

// ========== 远程攻击实现 ==========
void CombatSystem::rangeAttack(Player* attacker, const QPointF& direction)
{
    if (!attacker || attacker->isDead()) return;
    QPointF dir = direction;
    if(attacker->getTarget() && attacker->getTarget()->isAlive()){
        if(attacker->isLeftDirection()){
            dir.setY(dir.y() - (qreal)(attacker->getHeight() / 4));
        }else{
            dir.setY(dir.y() - (qreal)(attacker->getHeight() / 4));
        }
    }
    QPointF dirVec = getDirectionVector(dir);
    QPointF startPos = attacker->pos();
    if(attacker->isLeftDirection()){
        startPos.setY(startPos.y() + (qreal)(attacker->getHeight() / 4));
    }else{
        startPos.setY(startPos.y() + (qreal)(attacker->getHeight() / 4));
    }
    QPointF targetPos = startPos + dirVec * RANGE_ATTACK_RANGE;

    int totalDamage = calculateDamage(static_cast<int>(RANGE_DAMAGE_BASE),attacker->getTotalAttack());

    // ========== 使用 EntityManager 创建子弹 ==========
    Projectile* projectile = EntityManager::instance()->createEntity<Projectile>(
        startPos, targetPos, totalDamage, PROJECTILE_RADIUS, PROJECTILE_SPEED * 3,
        EntityTeam::PLAYER
        );

    EntityManager::instance()->addToScene(projectile);
    projectile->arrowIcon();
    emit projectileCreated(projectile);
    qDebug() << "Player range attack, projectile created, damage:" << totalDamage;
}
void CombatSystem::rangeAttack(Enemy* attacker, const QPointF& direction)
{
    if (!attacker || attacker->isDead()) return;

    QPointF dirVec = getDirectionVector(direction);
    QPointF startPos = attacker->pos();
    QPointF targetPos = startPos + dirVec * RANGE_ATTACK_RANGE;

    // ========== 使用 EntityManager 创建子弹 ==========
    Projectile* projectile = EntityManager::instance()->createEntity<Projectile>(
        startPos, targetPos,attacker->getDamage(),
        PROJECTILE_RADIUS * 2, PROJECTILE_SPEED,
        EntityTeam::ENEMY
        );
    projectile->fireballIcon();
    EntityManager::instance()->addToScene(projectile);
    emit projectileCreated(projectile);
    qDebug() << "Enemy range attack, projectile created at:" << startPos;
}

// ========== 技能接口 ==========
void CombatSystem::castSkill(int skillId, Player* caster, const QPointF& targetPos)
{
    if (!m_skills.contains(skillId)) {
        qDebug() << "Unknown skill ID:" << skillId;
        return;
    }

    SkillInfo& skill = m_skills[skillId];

    if (caster->getMana() < skill.manaCost) {
        qDebug() << "Not enough mana to cast" << skill.name;
        return;
    }

    caster->setMana(caster->getMana() - skill.manaCost);
    emit skillCast(skillId, targetPos);

    if (skill.type == "area") {
        qDebug() << "Area damage skill cast at" << targetPos;
    }
}

void CombatSystem::applyAreaDamage(const QPointF& center, int radius, int damage, QVector<Enemy*>& enemies)
{
    Q_UNUSED(center);
    Q_UNUSED(radius);
    Q_UNUSED(damage);
    Q_UNUSED(enemies);
}

// ========== 碰撞检测接口 ==========
bool CombatSystem::checkCollision(const QRectF& rect1, const QRectF& rect2)
{
    return rect1.intersects(rect2);
}

QVector<Enemy*> CombatSystem::getEnemiesInArea(const QPointF& center, int radius, const QVector<Enemy*>& enemies)
{
    QVector<Enemy*> result;
    qreal radiusSq = static_cast<qreal>(radius) * radius;

    for (Enemy* enemy : enemies) {
        if (enemy && !enemy->isDead()) {
            QPointF delta = center - enemy->pos();
            qreal distSq = delta.x() * delta.x() + delta.y() * delta.y();
            if (distSq <= radiusSq) {
                result.append(enemy);
            }
        }
    }
    return result;
}

QVector<Player*> CombatSystem::getPlayersInArea(const QPointF& center, qreal radius, Player* excludePlayer)
{
    Q_UNUSED(center);
    Q_UNUSED(radius);
    Q_UNUSED(excludePlayer);
    return QVector<Player*>();
}

// ========== 效果接口 ==========
void CombatSystem::applyKnockback(QGraphicsItem* target, const QPointF& direction, qreal force)
{
    Q_UNUSED(target);
    Q_UNUSED(direction);
    Q_UNUSED(force);
}

int CombatSystem::calculateDamage(int baseDamage,int weaponBonus)
{
    return baseDamage + weaponBonus;
}

void CombatSystem::castFireball(Player* attacker){
    if (!attacker) return;
    QPointF dir = attacker->getAttackDirection();
    QPointF dirVec = getDirectionVector(dir);
    QPointF startPos = attacker->pos();
    QPointF targetPos = startPos + dirVec * RANGE_ATTACK_RANGE;

    int totalDamage = calculateDamage(Item::FIREBALL_ATTACK,attacker->getTotalAttack());

    // ========== 使用 EntityManager 创建子弹 ==========
    Projectile* projectile = EntityManager::instance()->createEntity<Projectile>(
        startPos, targetPos, totalDamage, PROJECTILE_RADIUS * 2, PROJECTILE_SPEED,
        EntityTeam::PLAYER
        );

    EntityManager::instance()->addToScene(projectile);
    projectile->fireballIcon();
    emit projectileCreated(projectile);
    qDebug() << "Player cast fireball, fireball created, damage:" << totalDamage;
}
