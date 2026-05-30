#ifndef PET_H
#define PET_H

#include <QObject>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QPixmap>
#include <QMap>
#include <QPointF>
#include <QHash>
#include "Entity.h"

class Player;
class Enemy;

class Pet : public Entity, public QGraphicsPixmapItem
{
    Q_OBJECT
    DECLARE_ENTITY_TYPE(Pet, EntityType::PET)

public:
    // ========== 宠物类型枚举 ==========
    enum PetType {
        PET_CAT,
        PET_FROG,
        PET_PANDA,
        PET_PICAQU
    };

    // ========== 宠物稀有度 ==========
    enum PetRarity {
        RARITY_COMMON,     // 普通
        RARITY_RARE,       // 稀有
        RARITY_EPIC,       // 史诗
        RARITY_LEGENDARY   // 传说
    };
    Pet(PetType type, int level = 1, QGraphicsItem* parent = nullptr);
    ~Pet();

    // ========== 实现 Entity 的纯虚函数 ==========
    QGraphicsItem* graphicsItem() override { return this; }
    const QGraphicsItem* graphicsItem() const override { return this; }

    // ========== 重写 Entity 的方法 ==========
    void update(float deltaTime) override;
    void takeDamage(int damage, Entity* source = nullptr) override;

    // ========== 尺寸接口 ==========
    int getWidth() { return m_width; }
    int getHeight() { return m_height; }
    int getHalfWidth() { return m_width / 2; }
    int getHalfHeight() { return m_height / 2; }
    int getCollisionWidth() { return (qreal)m_width * m_zoom; }
    int getCollisionHeight() { return (qreal)m_height * m_zoom; }


    // ========== 基础属性接口 ==========
    PetType getPetType() const { return m_type; }
    QString getTypeName() const;
    PetRarity getRarity() const { return m_rarity; }

    int getLevel() const { return m_level; }
    int getAttack() const { return m_attack; }
    int getDefense() const { return m_defense; }
    qreal getAttackRange() const { return m_attackRange; }
    qreal getAttackCooldown() const { return m_attackCooldownMax; }
    qreal getFollowDistance() const { return m_followDistance; }

    void setPetActive(bool active);
    void revive();

    // ========== 升级接口 ==========
    void addExperience(int exp);
    int getExperience() const { return m_experience; }
    int getNextLevelExp() const { return m_nextLevelExp; }
    void levelUp();

    // ========== 行为接口 ==========
    void followOwner(Player* owner);
    Enemy* findNearestEnemy() const;
    QPointF getAttackDirection() const { return m_attackDirection; }

    // ========== 动画控制接口 ==========
    void playAnimation(const QString& name, bool loop = true);
    void stopAnimation();
    void setAnimationFps(int fps) { m_animationFps = fps; }
    void setPetOpacity(qreal opacity);

signals:
    void petDamaged(Pet* pet, int damage);
    void petHealed(Pet* pet, int amount);
    void petLeveledUp(Pet* pet, int newLevel);
    void petAttackEnemy(Pet* pet, Enemy* target, int damage);
    void petDied(Pet* pet);
    void petRevived(Pet* pet);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private slots:
    void updateAnimationFrame();

private:
    void catAnimation();//猫动画
    void frogAnimation();
    void picaquAnimation();

    //宠物技能
    void castLightning();

    // ========== 宠物属性 ==========
    PetRarity m_rarity;
    QString m_name;

    int m_level;
    int m_experience;
    int m_nextLevelExp;
    int m_attack;
    int m_defense;

    qreal m_followDistance;
    qreal m_attackRange;
    int m_attackCooldownMax;
    int m_attackCooldown;
    int m_skillCooldown;
    int m_skillCooldownMax = 500;
    QString m_attackType;
    qreal m_moveSpeed;

    QPointF m_attackDirection;
    Enemy* m_currentTarget;
    QString m_currentAnimation;
    QString m_currentDirection;
    QTimer* m_animationTimer;
    int m_animationFps;
    QPixmap m_currentPixmap;
    QPixmap m_defaultPixmap;

    // ========== 私有方法 ==========
    void startAnimationTimer();
    void stopAnimationTimer();
    void updateCurrentFrame();
    void initPetAttributes();
    void setMoveDirection(const QPointF& velocity);
    QMap<QString, AnimationData> m_animations;

    int m_width;
    int m_height;
    int m_originalDir;
    bool m_reverse = false;
    void (Pet::*m_skillPointer)(void);//技能指针
    qreal m_zoom;
    PetType m_type;

    static int LEFT;
    static int RIGHT;

    static int CAT_WIDTH;
    static int CAT_HEIGHT;
    static qreal CAT_ZOOM;

    static int FROG_WIDTH;
    static int FROG_HEIGHT;
    static qreal FROG_ZOOM;

    static int PICAQU_WIDTH;
    static int PICAQU_HEIGHT;
    static qreal PICAQU_ZOOM;
};

#endif // PET_H
