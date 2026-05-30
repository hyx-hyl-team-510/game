#ifndef ENTITY_H
#define ENTITY_H

#include <QObject>
#include <QPointF>
#include <QGraphicsItem>

// ========== 实体类型枚举 ==========
enum class EntityType {
    NONE = -1,
    PLAYER,
    ENEMY,
    PET,
    PROJECTILE,
    ITEM,
    EFFECT,
    WALL,
    TRIGGER,
    NPC,
    INTERACTIVE_OBJECT
};

// ========== 实体阵营 ==========
enum class EntityTeam {
    NEUTRAL,
    PLAYER,
    ENEMY
};

// ========== 动画 ==========
struct AnimationData {
    QPixmap spriteSheet;
    int frameWidth;
    int frameHeight;
    int validFrames;
    int cols;
    int currentFrame;
    int attackCallbackFrame;//攻击回调帧
    bool loop;
    AnimationData(){}
    AnimationData(QString path,int fw,int fh,int vf,int cs,bool lp){
        spriteSheet.load(path);
        frameWidth = fw;
        frameHeight = fh;
        validFrames = vf;
        cols = cs;
        currentFrame = 0;
        attackCallbackFrame = -1;
        loop = lp;
    }
};

// ========== 实体基类（纯数据，不继承图形类） ==========
class Entity : public QObject {
    Q_OBJECT

public:
    Entity(EntityType type, QObject* parent = nullptr);
    virtual ~Entity();

    // 子类必须实现：返回自己的图形项
    virtual QGraphicsItem* graphicsItem() = 0;
    virtual const QGraphicsItem* graphicsItem() const = 0;

    // ========== 便捷方法（转发到图形项） ==========
    QPointF pos() const;
    void setPos(const QPointF& pos);
    void setPos(qreal x, qreal y);
    qreal x() const;
    qreal y() const;
    QGraphicsScene* scene() const;
    bool isVisible() const;
    void setVisible(bool visible);
    void update();

    // ========== 核心属性 ==========
    EntityType getType() const { return m_type; }
    EntityTeam getTeam() const { return m_team; }
    void setTeam(EntityTeam team);

    uint64_t getUniqueId() const { return m_uniqueId; }

    bool isActive() const { return m_isActive; }
    void setActive(bool active);

    bool isAlive() const { return m_health > 0; }
    bool isDead() const { return m_health <= 0; }

    // ========== 生命周期 ==========
    virtual void onSpawn() {}
    virtual void onDestroy() {}
    virtual void update(float deltaTime) { Q_UNUSED(deltaTime); }

    // ========== 碰撞 ==========
    virtual QRectF getCollisionRect() const;
    virtual bool canCollideWith(Entity* other) const;
    virtual void onCollision(Entity* other) { Q_UNUSED(other); }

    // ========== 属性 ==========
    int getHealth() const { return m_health; }
    int getMaxHealth() const { return m_maxHealth; }
    int getHealthPercent() const { return (double)m_health / (double)m_maxHealth * 100.0;}
    void setMaxHealth(int maxHealth);
    virtual void takeDamage(int damage, Entity* source = nullptr);
    virtual void heal(int amount);

    // ========== 位置/移动 ==========
    QPointF getVelocity() const { return m_velocity; }
    void setVelocity(const QPointF& v) { m_velocity = v; }

signals:
    void entityDestroyed(Entity* entity);
    void healthChanged(Entity* entity, int oldHealth, int newHealth);
    void damageTaken(Entity* entity, int damage, Entity* source);
    void entityHealed(Entity* entity, int amount);
    void teamChanged(Entity* entity, EntityTeam oldTeam, EntityTeam newTeam);

protected:
    EntityType m_type;
    EntityTeam m_team = EntityTeam::NEUTRAL;
    uint64_t m_uniqueId;
    bool m_isActive = true;

    int m_health = 100;
    int m_maxHealth = 100;
    QPointF m_velocity;

    static uint64_t s_nextId;
};

// ========== 获取实体静态类型的辅助宏 ==========
#define DECLARE_ENTITY_TYPE(Type, typeEnum) \
public: \
    static EntityType staticEntityType() { return typeEnum; }

#endif // ENTITY_H
