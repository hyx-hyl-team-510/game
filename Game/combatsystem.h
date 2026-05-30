#ifndef COMBATSYSTEM_H
#define COMBATSYSTEM_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPointF>
#include <QRectF>
#include <QString>
#include "entitymanager.h"
// 前向声明
class QGraphicsItem;
class Player;
class Enemy;
class Projectile;

class CombatSystem : public QObject
{
    Q_OBJECT

public:
    explicit CombatSystem(QObject* parent = nullptr);
    // ========== 战斗动作接口 ==========
    // 近战攻击（玩家攻击敌人）
    void meleeAttack(Player* attacker, const QPointF& direction,QVector<Enemy*> target);
    // 近战攻击（敌人攻击玩家）
    void meleeAttack(Enemy* attacker,Player *target);

    // 远程攻击（玩家攻击敌人）
    void rangeAttack(Player* attacker, const QPointF& direction);
    // 远程攻击（敌人攻击玩家）
    void rangeAttack(Enemy* attacker, const QPointF& direction);
    // 技能接口
    void castSkill(int skillId, Player* caster, const QPointF& targetPos);
    void applyAreaDamage(const QPointF& center, int radius, int damage, QVector<Enemy*>& enemies);
    void castFireball(Player* attacker);//释放火球
    // 碰撞检测接口
    bool checkCollision(const QRectF& rect1, const QRectF& rect2);
    QVector<Enemy*> getEnemiesInArea(const QPointF& center, int radius, const QVector<Enemy*>& enemies);
    QVector<Player*> getPlayersInArea(const QPointF& center, qreal radius, Player* excludePlayer);

    // 效果接口
    void applyKnockback(QGraphicsItem* target, const QPointF& direction, qreal force);

signals:
    void playerDamageDealt(Player* source, Enemy* target, int damage);
    void enemyDamageDealt(Enemy* source, Player* target, int damage);
    void playerRangeDamageDealt(Player* source, Enemy* target, int damage);
    void enemyRangeDamageDealt(Enemy* source, Player* target, int damage);
    void skillCast(int skillId, const QPointF& position);
    void areaDamageApplied(const QPointF& center, int radius, int damage);
    void projectileCreated(Projectile* projectile);

private:
    // ========== 静态参数 ==========
    // 近战攻击参数
    static qreal MELEE_RANGE;           // 近战攻击范围半径
    static qreal MELEE_DAMAGE_BASE;     // 近战基础伤害
    static int MELEE_COOLDOWN;          // 近战攻击冷却帧数

    // 远程攻击参数
    static qreal RANGE_ATTACK_RANGE;    // 远程攻击最大距离
    static qreal PROJECTILE_SPEED;      // 子弹速度
    static qreal PROJECTILE_RADIUS;     // 子弹半径（攻击范围）
    static int RANGE_COOLDOWN;          // 远程攻击冷却帧数
    static qreal RANGE_DAMAGE_BASE;     // 远程基础伤害

    // 辅助函数
    QPointF getDirectionVector(const QPointF& direction);
    QVector<Enemy*> getEnemiesInCircularArea(const QPointF& center, qreal radius);
    QVector<Player*> getPlayersInCircularArea(const QPointF& center, qreal radius, Player* excludePlayer);

    // 技能配置
    struct SkillInfo {
        int id;
        QString name;
        int damage;
        int manaCost;
        int cooldown;
        int range;
        int areaRadius;
        QString type;
    };

    QMap<int, SkillInfo> m_skills;
    void initializeSkills();
    int calculateDamage(int baseDamage,int weaponBonus = 0);
};

#endif // COMBATSYSTEM_H
