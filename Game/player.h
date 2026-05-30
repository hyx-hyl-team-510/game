#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsPixmapItem>
#include <QObject>
#include <QTimer>
#include <QPixmap>
#include <QMap>
#include <QList>
#include "Entity.h"
#include "item.h"

class Weapon;  // 前向声明
class Enemy;
class Player : public Entity, public QGraphicsPixmapItem
{
    Q_OBJECT
    DECLARE_ENTITY_TYPE(Player, EntityType::PLAYER)

public:
    Player(qreal x, qreal y, QGraphicsItem* parent = nullptr);
    ~Player();

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

    // ========== 运动参数静态变量 ==========
    static qreal getMaxSpeed() { return MAX_SPEED; }
    static qreal getAcceleration() { return ACCELERATION; }
    static qreal getFriction() { return FRICTION; }

    static void setMaxSpeed(qreal speed) { MAX_SPEED = speed; }
    static void setAcceleration(qreal acc) { ACCELERATION = acc; }
    static void setFriction(qreal friction) { FRICTION = friction; }

    // ========== 基础属性接口 ==========
    int getMana() const { return m_mana; }
    int getMaxMana() const { return m_maxMana; }
    int getManaPercent() const { return (double) m_mana / (double)m_maxMana * 100.0; }
    qreal getMoveSpeed() const { return m_moveSpeed; }

    void setHealth(int health);
    void setMana(int mana);

    // ========== 移动接口 ==========
    void setKeyState(int key, bool pressed);
    void updateMovement();
    void setPosition(const QPointF& pos);

    // 获取速度相关
    QPointF getVelocity() const { return m_velocity; }
    QPointF getLastMoveDirection() const { return m_lastMoveDirection; }

    // ========== 攻击接口 ==========
    void attack();
    void timeStopAttack();
    void setAttackDirection(QPointF dir);//设置攻击方向
    QPointF getAttackDirection(){return m_attackDirection;}
    void setTarget(Enemy* target);
    Enemy* getTarget(){return m_target;}
    // ========== 碰撞接口 ==========
    QPointF getCenterPos(){return pos() + QPointF(m_width / 2,m_height / 2);}
    QRectF getCollisionRect() const override {
        return QRectF(pos().x() - m_width / 2.0 * m_zoom,
                      pos().y() - m_height / 2.0 * m_zoom,
                      m_width * m_zoom, m_height * m_zoom);
    }
    // ========== 精灵表动画接口 ==========
    void loadAnimation(const QString& name, const QString& imagePath,
                       int frameWidth, int frameHeight, int validFrameCount);
    void playAnimation(const QString& name, bool loop = true);
    void stopAnimation();
    void setAnimationFps(int fps) { m_animationFps = fps; }
    void setMoveDirection(const QString& direction);
    void setPlayerOpacity(qreal opacity);

    // ========== 金币 ==========
    int getGold() const { return m_gold; }
    void setGold(int gold) { m_gold = gold;emit updateGold(); }
    void addGold(int amount) { m_gold += amount; emit updateGold();}
    bool spendGold(int amount) {
        if (m_gold >= amount) {
            m_gold -= amount;
            emit updateGold();
            return true;
        }
        return false;
    }

    // ========== 武器背包系统 ==========

    // 容量管理
    int getWeaponCapacity() const { return m_weaponCapacity; }
    void setWeaponCapacity(int capacity);
    void upgradeWeaponCapacity(int additionalSlots = 1);
    int getMaxWeaponCapacity() const { return m_maxWeaponCapacity; }
    void setMaxWeaponCapacity(int max) { m_maxWeaponCapacity = max; }

    // 武器数量
    int getOwnedWeaponCount() const;
    bool isWeaponInventoryFull() const;

    // 武器管理
    bool addWeapon(Weapon* weapon);
    bool removeWeapon();//除去当前武器
    QList<Weapon*> getOwnedWeapons() const { return m_ownedWeapons; }

    // 装备管理
    void equipWeapon(Weapon* weapon);
    void unequipWeapon();
    Weapon* getCurrentWeapon() const { return m_currentWeapon; }
    bool switchToNextWeapon();

    // 攻击力
    int getTotalAttack() const;
    void setBaseAttack(int attack) { m_baseAttack = attack; }
    qreal getKnockback(){return m_knockback;}
    bool isLeftDirection(){return isLeft;}//获取方向
    bool isRightDirection(){return isRight;}
    bool isAttacking(){return isAttack;}
    ItemData& getItems(){return items;}//返回物品
    ItemData getItems()const{return items;}//返回物品
    QPointF getAnotherPos(){return m_anotherPos;}
    void setAnotherPos(QPointF p){m_anotherPos = p;}
    int getSkillCoolDown() {return m_skillCoolDown;}
    int getMaxSkillCoolDown() {return m_maxSkillCoolDown;}
    void resetSkillCoolDown() {m_skillCoolDown = m_maxSkillCoolDown;}
signals:
    void weaponInventoryChanged();
    void weaponEquipped(Weapon* weapon);
    void weaponUnequipped();
    void weaponCapacityUpgraded(int newCapacity);
    void performAttack();//动画末释放攻击
    void updateGold();

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private slots:
    void updateAnimationFrame();

private:
    void startAnimationTimer();
    void stopAnimationTimer();
    void updateCurrentFrame();
    void initPlayerAttributes();

    // ========== 尺寸常量 ==========
    static constexpr int PLAYER_WIDTH = 80;
    static constexpr int PLAYER_HEIGHT = 80;

    // ========== 运动参数静态变量 ==========
    static qreal MAX_SPEED;
    static qreal ACCELERATION;
    static qreal FRICTION;

    // ========== 基础属性 ==========
    int m_mana;
    int m_maxMana;
    qreal m_moveSpeed;
    // ========== 运动状态 ==========
    QPointF m_velocity;
    QPointF m_lastMoveDirection;
    bool m_keyUp, m_keyDown, m_keyLeft, m_keyRight;
    QMap<QString, AnimationData> m_animations;
    QString m_currentAnimation;
    QString m_currentDirection;
    bool m_useAnimation;
    QTimer* m_animationTimer;
    int m_animationFps;
    QPixmap m_currentPixmap;
    int m_gold;
    QPointF m_anotherPos;//另一个世界的位置

    // ========== 武器背包 ==========
    Weapon* m_currentWeapon;
    QList<Weapon*> m_ownedWeapons;
    int m_weaponCapacity;
    int m_maxWeaponCapacity;
    QString getWeaponIdleName() const;  // 根据武器获取 idle 动画名
    QString getWeaponWalkName(const QString& direction); // 根据武器获取 walk 动画名
    int m_baseAttack;
    int m_width;
    int m_height;
    qreal m_zoom;
    qreal m_knockback = 50;//击退能力
    int m_skillCoolDown = 0;
    int m_maxSkillCoolDown = 1000;

    bool isLeft = false;//左移
    bool isRight = true;//右移
    bool isAttack = false;//正在攻击
    qreal minAutoAimRadius;//最小自动瞄准半径，大于这个值才会自动瞄准
    QPointF m_attackDirection;//攻击方向
    Enemy* m_target = nullptr;//记录当前锁定的敌人
    QPointF getPlayerAttackDirection();//获取玩家移动方向，作为攻击方向
    void updateAutoAim();//更新自动瞄准方向
    ItemData items;//道具
    int baseManaCost = 1;//基础魔力消耗
    void switchWeaponSound();//换武器音效

};

#endif // PLAYER_H
