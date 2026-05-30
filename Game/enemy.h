#ifndef ENEMY_H
#define ENEMY_H
#include <QRandomGenerator>
#include <QGraphicsRectItem>
#include <QObject>
#include <QPointF>
#include <QColor>
#include <QTimer>
#include <QPixmap>
#include <QMap>
#include <QDebug>
#include "Entity.h"

class Player;
class CombatSystem;
class Enemy : public Entity, public QGraphicsRectItem
{
    Q_OBJECT
    DECLARE_ENTITY_TYPE(Enemy, EntityType::ENEMY)

public:
    static CombatSystem* combatSystem;
    enum EnemyType {
        TYPE_NORMAL,
        TYPE_ELITE,
        TYPE_BOSS
    };

    enum AttackType{
        MELEE,
        RANGE
    };

    Enemy(qreal x, qreal y, int level, QGraphicsItem* parent = nullptr);
    ~Enemy();

    // ========== 实现 Entity 的纯虚函数 ==========
    QGraphicsItem* graphicsItem() override { return this; }
    const QGraphicsItem* graphicsItem() const override { return this; }

    // ========== 重写 Entity 的方法 ==========
    void update(float deltaTime) override;
    void takeDamage(int damage, Entity* source = nullptr) override;

    // ========== 消除歧义 ==========
    using Entity::isActive;
    using Entity::setActive;
    using Entity::isAlive;
    using Entity::isDead;
    using Entity::getHealth;
    using Entity::getMaxHealth;
    using Entity::pos;
    using Entity::setPos;
    using Entity::x;
    using Entity::y;
    using Entity::scene;
    using Entity::setVisible;
    using Entity::isVisible;

    // ========== 尺寸接口 ==========
    int getWidth() { return m_width; }
    int getHeight() { return m_height; }
    int getHalfWidth() { return m_width / 2; }
    int getHalfHeight() { return m_height / 2; }
    qreal getCollisionWidth(){return m_width * m_zoom;}
    qreal getCollisionHeight(){return m_height * m_zoom;}
    // ========== 属性接口 ==========
    EnemyType getEnemyType() const { return m_enemyType; }
    int getDamage() const { return m_damage; }
    int getLevel() const { return m_level; }

    // ========== 行为接口 ==========
    void avoidOtherEnemies();
    void moveTowards(const QPointF& target, qreal speed);

    // ========== 攻击接口 ==========
    void attack(Player* player);
    void meleeAttack(Player* player);
    void rangeAttack(Player* player);
    QPointF getAttackDirection() const { return m_attackDirection; }
    void setAttackDirection(const QPointF& direction) { m_attackDirection = direction; }
    // ========== 碰撞接口 ==========
    QRectF getCollisionRect() const override {
        return QRectF(pos().x() - m_width / 2.0 * m_zoom,
                      pos().y() - m_height / 2.0 * m_zoom,
                      m_width * m_zoom, m_height * m_zoom);
    }
    void knockback(QPointF dir,qreal speed);//击退函数
    // ========== 精灵表动画接口 ==========
    void playAnimation(const QString& name, bool loop = true);
    void stopAnimation();
    void setAnimationFps(int fps) { m_animationFps = fps; }
    void setMoveDirection(const QPointF& velocity);

    // 设置透明度（用于受伤闪烁）
    void setEnemyOpacity(qreal opacity);
    void kill();//代码层面删除
    //自动配置敌人动画
    void blueSlimeAnimation();
    void greenSlimeAnimation();
    void flyingMonsterAnimation();
    void wizardAnimation();
    void assassinAnimation();
    void soulAnimation();

signals:

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private slots:
    void updateAnimationFrame();

private:
    void startAnimationTimer();
    void stopAnimationTimer();
    void updateCurrentFrame();
    void initEnemyAttributes();
    void bossSkill();//boss技能
    // ========== 基础属性 ==========

    EnemyType m_enemyType;
    AttackType m_attackType;

    QPointF m_attackDirection;
    QPointF m_knockbackV;//击退速度

    QMap<QString, AnimationData> m_animations;
    QString m_currentAnimation;
    QString m_currentDirection;
    QTimer* m_animationTimer;
    QPixmap m_currentPixmap;

    bool m_useAnimation;
    bool m_reverse = false;//图片绘制是否反向

    int m_animationFps;
    int m_damage;
    int m_level;
    int m_goldReward;
    int m_width;
    int m_height;
    int m_originalDir;
    int m_attackCooldown;
    int m_maxAttackCooldown;
    int m_bossCooldown = 1000;//boss的攻击范围更新冷却
    int m_maxBossCooldown = 1000;
    int m_knockbackCnt = 0;//击退次数
    int m_maxKnockbackCnt = 25;//最大击退次数

    qreal autoRadius = 400;//自动追踪距离
    qreal m_attackRadius;
    qreal m_zoom;
    qreal m_moveSpeed;

    static int BLUE_SLIME_WIDTH;
    static int BLUE_SLIME_HEIGHT;
    static int GREEN_SLIME_WIDTH;
    static int GREEN_SLIME_HEIGHT;
    static int FLYING_MONSTER_WIDTH;
    static int FLYING_MONSTER_HEIGHT;
    static int WIZARD_WIDTH;
    static int WIZARD_HEIGHT;
    static int ASSASSIN_WIDTH;
    static int ASSASSIN_HEIGHT;
    static int SOUL_WIDTH;
    static int SOUL_HEIGHT;
    static int LEFT;
    static int RIGHT;
    static int MELEE_COOLDOWN;//攻击冷却
    static int RANGE_COOLDOWN;

    static qreal BLUE_SLIME_ZOOM;
    static qreal GREEN_SLIME_ZOOM;
    static qreal FLYING_MONSTER_ZOOM;
    static qreal WIZARD_ZOOM;
    static qreal ASSASSIN_ZOOM;
    static qreal SOUL_ZOOM;
    static qreal MELEE_ATTACK_RADIUS;//近战攻击范围
    static qreal RANGE_ATTACK_RADIUS;//远程攻击范围
};

#endif // ENEMY_H
