#include "effect.h"
#include "entitymanager.h"
#include "audiomanager.h"

Effect::Effect(QPointF p,EffectType type,QGraphicsItem* parent):
    Entity(EntityType::EFFECT),QGraphicsPixmapItem(parent)
{
    setTeam(EntityTeam::NEUTRAL);
    Entity::setActive(true);
    Entity::setPos(p);
    m_type = type;
    switch(m_type){
        case EXPLOSION:explosionEffect();break;
        case SLASH:slashEffect();break;
        case IMPACT:impactEffect();break;
        case LIGHTNING:lightningEffect();break;
        case HEAL:healEffect();break;
        case MANA:manaEffect();break;
    }
    qDebug() << "Effect created";
}
Effect::~Effect()
{
    qDebug() << "Effect destroyed";
}

void Effect::update(float deltaTime)
{
    Q_UNUSED(deltaTime);
    // 更新帧显示
    int col = m_anim.currentFrame % m_anim.cols;
    int row = m_anim.currentFrame / m_anim.cols;
    QRect frameRect(col * m_anim.frameWidth, row * m_anim.frameHeight,
                    m_anim.frameWidth, m_anim.frameHeight);
    setPixmap(m_anim.spriteSheet.copy(frameRect));
    if(m_anim.currentFrame == m_anim.attackCallbackFrame && m_callback){
        m_callback();//执行回调
    }else if(m_anim.currentFrame == m_anim.validFrames - 1){
        Entity::setActive(false);
        EntityManager::instance()->destroyEntity(this);
    }
    return;
}

void Effect::explosionEffect(){
    static AnimationData explosion("resource/Effect/explosion.png",128,128,10,1,false);
    m_anim = explosion;
    connect(&animTimer,&QTimer::timeout,this,[this](){
        m_anim.currentFrame = (m_anim.currentFrame + 1) % m_anim.validFrames;
    });
    animTimer.start(1000 / 12);
    AudioManager::instance()->playSound(AudioManager::BOOM);
    return;
}

void Effect::slashEffect(){
    static AnimationData explosion("resource/Effect/slashTail/slash_3.png",128,108,4,4,false);
    Entity::setPos(Entity::pos() - QPointF(64,52));
    m_anim = explosion;
    connect(&animTimer,&QTimer::timeout,this,[this](){
        m_anim.currentFrame = (m_anim.currentFrame + 1) % m_anim.validFrames;
    });
    animTimer.start(1000 / 12);
    return;
}

void Effect::impactEffect(){
    static AnimationData impact("resource/Effect/impact/impact.png",128,128,12,4,false);
    Entity::setPos(Entity::pos() - QPointF(64,64));
    m_anim = impact;
    connect(&animTimer,&QTimer::timeout,this,[this](){
        m_anim.currentFrame = (m_anim.currentFrame + 1) % m_anim.validFrames;
    });
    animTimer.start(1000 / 12);
    return;
}

void Effect::lightningEffect(){
    AudioManager::instance()->playSound(AudioManager::LIGHTNING);
    static AnimationData lightning("resource/Effect/lightning/lightning.png",105,102,16,16,false);
    lightning.attackCallbackFrame = 6;
    Entity::setPos(Entity::pos() - QPointF(53,90));
    m_anim = lightning;
    connect(&animTimer,&QTimer::timeout,this,[this](){
        m_anim.currentFrame = (m_anim.currentFrame + 1) % m_anim.validFrames;
    });
    animTimer.start(1000 / 12);
    return;
}

void Effect::healEffect(){
    AudioManager::instance()->playSound(AudioManager::HEAL);
    static AnimationData heal("resource/Effect/heal/heal.png",167,167,5,5,false);
    m_anim = heal;
    Entity::setPos(Entity::pos() - QPointF(84,84));
    connect(&animTimer,&QTimer::timeout,this,[this](){
        m_anim.currentFrame = (m_anim.currentFrame + 1) % m_anim.validFrames;
    });
    animTimer.start(1000 / 12);
    return;
}

void Effect::manaEffect(){
    AudioManager::instance()->playSound(AudioManager::HEAL);
    static AnimationData mana("resource/Effect/heal/mana.png",167,167,5,5,false);
    m_anim = mana;
    Entity::setPos(Entity::pos() - QPointF(84,84));
    connect(&animTimer,&QTimer::timeout,this,[this](){
        m_anim.currentFrame = (m_anim.currentFrame + 1) % m_anim.validFrames;
    });
    animTimer.start(1000 / 12);
    return;
}
