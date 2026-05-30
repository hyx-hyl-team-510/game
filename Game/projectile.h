#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <QGraphicsEllipseItem>
#include <QPointF>
#include "Entity.h"
#include <QString>
class Projectile : public Entity, public QGraphicsEllipseItem
{
    Q_OBJECT
    DECLARE_ENTITY_TYPE(Projectile, EntityType::PROJECTILE)

public:
    enum ProjectileType{
        ARROW,
        FIREBALL,
        LASER
    };
    Projectile(const QPointF& startPos, const QPointF& targetPos,
               int damage, qreal radius, qreal speed,
               EntityTeam team = EntityTeam::PLAYER,
               QGraphicsItem* parent = nullptr);
    ~Projectile();

    // ========== 实现 Entity 的纯虚函数 ==========
    QGraphicsItem* graphicsItem() override { return this; }
    const QGraphicsItem* graphicsItem() const override { return this; }

    // ========== 重写 Entity 的方法 ==========
    void update(float deltaTime) override;

    // ========== 消除歧义 ==========
    using Entity::isActive;
    using Entity::setActive;
    using Entity::pos;
    using Entity::setPos;
    using Entity::x;
    using Entity::y;

    // ========== 属性接口 ==========
    int getDamage() const { return m_damage; }
    bool isProjectileActive() const { return m_active; }
    void updateEffect();
    void setProjectileActive(bool active) { m_active = active; }
    void setAngle(qreal angle){ m_angle = angle; }
    QPointF getCurrentPos() const { return pos(); }
    qreal getRadius() const { return m_radius; }
    qreal getAngle(){return m_angle;}

    //投掷物图标
    void arrowIcon();
    void fireballIcon();
    void laserIcon();
protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    QPointF m_direction;
    QPointF m_pos;
    int m_damage;
    qreal m_speed;
    bool m_active;
    qreal m_distanceTraveled;
    qreal m_radius;
    QPixmap m_pixmap;  // 子弹图片
    bool m_usePixmap = false;  // 是否使用图片
    bool m_useAnimation = false;//是否使用动画
    qreal m_angle;  // 旋转角度（弧度）
    AnimationData anim;
    QTimer *animTimer = nullptr;
    ProjectileType m_type;

};

#endif // PROJECTILE_H
