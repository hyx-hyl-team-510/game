#ifndef EFFECT_H
#define EFFECT_H
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QDebug>
#include <QTimer>
#include <QString>
#include <functional>
#include "entity.h"

class Effect : public Entity,public QGraphicsPixmapItem
{
    Q_OBJECT
    DECLARE_ENTITY_TYPE(Effect, EntityType::EFFECT)
public:
    enum EffectType{//特效种类
        EXPLOSION,
        SLASH,
        IMPACT,
        LIGHTNING,
        HEAL,
        MANA
    };
    Effect(QPointF p,EffectType type,QGraphicsItem* parent = nullptr);
    ~Effect();
    QGraphicsItem* graphicsItem() override { return this; }
    const QGraphicsItem* graphicsItem() const override { return this; }
    void update(float deltaTime) override;
    void setCallback(std::function<void()> callback) {m_callback = callback;}
private:
    void explosionEffect();
    void slashEffect();
    void impactEffect();
    void lightningEffect();
    void healEffect();
    void manaEffect();
    AnimationData m_anim;
    QTimer animTimer;
    EffectType m_type;
    std::function<void()> m_callback = nullptr;//攻击回调

};

#endif // EFFECT_H
