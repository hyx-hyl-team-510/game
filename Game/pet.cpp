#include "Pet.h"
#include "Player.h"
#include "Enemy.h"
#include "effect.h"
#include "EntityManager.h"
#include <QPainter>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <cmath>
#include <QPointer>
#include <QVector>
#include <functional>

int Pet::LEFT = -1;
int Pet::RIGHT = 1;

int Pet::CAT_WIDTH = 30;
int Pet::CAT_HEIGHT = 30;
qreal Pet::CAT_ZOOM = 0.8;

int Pet::FROG_WIDTH = 30;
int Pet::FROG_HEIGHT = 30;
qreal Pet::FROG_ZOOM = 0.8;

int Pet::PICAQU_WIDTH = 48;
int Pet::PICAQU_HEIGHT = 32;
qreal Pet::PICAQU_ZOOM = 0.8;

// ========== 构造函数 ==========
Pet::Pet(PetType type, int level, QGraphicsItem* parent)
    : Entity(EntityType::PET)
    , QGraphicsPixmapItem(parent)
    , m_type(type)
    , m_level(level)
    , m_experience(0)
    , m_attackCooldown(0)
    , m_skillCooldown(0)
    , m_attackDirection(1.0, 0.0)
    , m_currentTarget(nullptr)
    , m_animationTimer(nullptr)
    , m_animationFps(10)
    , m_currentDirection("down")
    , m_skillPointer(nullptr)
{
    setTeam(EntityTeam::PLAYER);

    initPetAttributes();
    setMaxHealth(m_maxHealth);
    m_health = m_maxHealth;
    qDebug() << "Pet created:" << getTypeName() << "Level:" << m_level;
}

Pet::~Pet()
{
    stopAnimationTimer();
    qDebug() << "Pet destroyed";
}

void Pet::initPetAttributes()
{
    m_nextLevelExp = 100 + m_level * 20;

    switch (m_type) {
    case PET_CAT:
        m_name = "灵猫";
        m_rarity = RARITY_COMMON;
        m_maxHealth = 40 + m_level * 10;
        m_attack = 8 + m_level * 2;
        m_defense = 2 + m_level * 1;
        m_followDistance = 80.0;
        m_attackRange = 40.0;
        m_attackCooldownMax = 30;
        m_attackType = "melee";
        m_moveSpeed = 3.5;
        catAnimation();
        break;

    case PET_FROG:
        m_name = "青蛙";
        m_rarity = RARITY_COMMON;
        m_maxHealth = 50 + m_level * 12;
        m_attack = 10 + m_level * 2;
        m_defense = 3 + m_level * 1;
        m_followDistance = 70.0;
        m_attackRange = 35.0;
        m_attackCooldownMax = 25;
        m_attackType = "melee";
        m_moveSpeed = 4.0;
        frogAnimation();
        break;

    case PET_PANDA:
        m_name = "小熊猫";
        m_rarity = RARITY_RARE;
        m_maxHealth = 30 + m_level * 8;
        m_attack = 12 + m_level * 3;
        m_defense = 1 + m_level * 1;
        m_followDistance = 100.0;
        m_attackRange = 200.0;
        m_attackCooldownMax = 40;
        m_attackType = "melee";
        m_moveSpeed = 3.0;
        break;

    case PET_PICAQU:
        m_name = "皮卡丘";
        m_rarity = RARITY_EPIC;
        m_maxHealth = 60 + m_level * 15;
        m_attack = 15 + m_level * 4;
        m_defense = 5 + m_level * 2;
        m_followDistance = 90.0;
        m_attackRange = 180.0;
        m_attackCooldownMax = 45;
        m_attackType = "range";
        m_moveSpeed = 3.2;
        picaquAnimation();
        break;

    default:
        m_name = "未知";
        m_rarity = RARITY_COMMON;
        m_maxHealth = 30;
        m_attack = 5;
        m_defense = 1;
        m_followDistance = 80.0;
        m_attackRange = 40.0;
        m_attackCooldownMax = 30;
        m_attackType = "melee";
        m_moveSpeed = 3.0;
        break;
    }
}

// ========== 类型名称接口 ==========
QString Pet::getTypeName() const
{
    switch (m_type) {
    case PET_CAT:    return "灵猫";
    case PET_FROG:    return "青蛙";
    case PET_PANDA:  return "小熊猫";
    case PET_PICAQU: return "皮卡丘";
    default:         return "未知";
    }
}

// ========== 重写 Entity 的方法 ==========
void Pet::update(float deltaTime)
{
    Q_UNUSED(deltaTime);

    if (!Entity::isActive() || isDead()) return;

    // 从 EntityManager 获取玩家
    Entity* playerEntity = EntityManager::instance()->getPlayer();
    if (!playerEntity) return;

    Player* player = static_cast<Player*>(playerEntity);

    // 更新冷却
    if (m_attackCooldown > 0) {
        m_attackCooldown--;
    }
    if (m_skillCooldown > 0) {
        m_skillCooldown--;
    }else{
        if(m_skillPointer){
            (this->*m_skillPointer)();
            m_skillCooldown = m_skillCooldownMax;
        }
    }
    followOwner(player);
}

void Pet::takeDamage(int damage, Entity* source)
{
    Q_UNUSED(source);

    int actualDamage = qMax(1, damage - m_defense);

    // 调用基类方法
    Entity::takeDamage(actualDamage, nullptr);

    setPetOpacity(0.5);
    QPointer<Pet> self(this);
    QTimer::singleShot(100, [self]() {
        if (!self.isNull()) {
            self->setPetOpacity(1.0);
        }
    });

    emit petDamaged(this, actualDamage);

    if (isDead()) {
        Entity::setActive(false);
        emit petDied(this);
        qDebug() << "Pet died:" << getTypeName();
    }
}

// ========== 状态接口 ==========
void Pet::setPetActive(bool active)
{
    Entity::setActive(active);
    Entity::setVisible(active);
}

void Pet::revive()
{
    m_health = m_maxHealth;
    Entity::setActive(true);
    Entity::setVisible(true);
    emit petRevived(this);
    qDebug() << "Pet revived:" << getTypeName();
}

// ========== 升级接口 ==========
void Pet::addExperience(int exp)
{
    m_experience += exp;
    while (m_experience >= m_nextLevelExp && m_level < 50) {
        levelUp();
    }
}

void Pet::levelUp()
{
    m_experience -= m_nextLevelExp;
    m_level++;
    m_nextLevelExp = static_cast<int>(m_nextLevelExp * 1.2);

    int oldMaxHealth = m_maxHealth;
    initPetAttributes();
    m_health = m_maxHealth;
    setMaxHealth(m_maxHealth);

    emit petLeveledUp(this, m_level);
    qDebug() << "Pet leveled up:" << getTypeName() << "Level:" << m_level;
}

// ========== 行为接口 ==========
void Pet::followOwner(Player* owner)
{
    if (!owner) return;

    QPointF delta = owner->pos() - Entity::pos();
    qreal distance = std::sqrt(delta.x() * delta.x() + delta.y() * delta.y());

    if (distance > m_followDistance) {
        QPointF direction = delta / distance;
        QPointF movement = direction * m_moveSpeed;
        Entity::setPos(Entity::pos() + movement);
        setMoveDirection(movement);
    } else if (distance < m_followDistance * 0.5) {
        QPointF direction = delta / distance;
        QPointF movement = -direction * m_moveSpeed * 0.5;
        Entity::setPos(Entity::pos() + movement);
        setMoveDirection(movement);
    } else {
        setMoveDirection(QPointF(0, 0));
    }
}

Enemy* Pet::findNearestEnemy() const
{
    Entity* nearest = EntityManager::instance()->getNearestEnemy(Entity::pos(), m_attackRange * 1.5);
    return static_cast<Enemy*>(nearest);
}

// ========== 动画接口 ==========
void Pet::playAnimation(const QString& name, bool loop)
{
    if (!m_animations.contains(name)) return;

    if (m_currentAnimation == name && m_animationTimer && m_animationTimer->isActive()) {
        return;
    }

    AnimationData& anim = m_animations[name];
    anim.currentFrame = 0;
    anim.loop = loop;
    m_currentAnimation = name;

    updateCurrentFrame();
    startAnimationTimer();
}

void Pet::stopAnimation()
{
    stopAnimationTimer();
    m_currentAnimation = "";
}

void Pet::setMoveDirection(const QPointF& velocity)
{
    QString direction;

    if (velocity.x() == 0 && velocity.y() == 0) {
        direction = "idle";
    } else{
        m_reverse = (velocity.x() * m_originalDir > 0) ? false : true;
        direction = "walk";
    }

    if (m_currentDirection == direction) return;
    m_currentDirection = direction;

    QString animationName;
    if (direction == "idle"){
        animationName = "idle";
    }else{
        animationName = "walk";
    }

    if (!animationName.isEmpty() && m_animations.contains(animationName)) {
        playAnimation(animationName, true);
    } else if (m_animations.contains("idle")) {
        playAnimation("idle", true);
    }
}

void Pet::setPetOpacity(qreal opacity)
{
    QGraphicsPixmapItem::setOpacity(opacity);
}

// ========== 私有动画方法 ==========
void Pet::updateAnimationFrame()
{
    if (!m_animations.contains(m_currentAnimation)) return;

    AnimationData& anim = m_animations[m_currentAnimation];
    anim.currentFrame++;

    if (anim.currentFrame >= anim.validFrames) {
        if (anim.loop) {
            anim.currentFrame = 0;
        } else {
            stopAnimation();
            if (m_animations.contains("idle")) {
                playAnimation("idle",true);
            }
            return;
        }
    }

    updateCurrentFrame();
}

void Pet::updateCurrentFrame()
{
    if (!m_animations.contains(m_currentAnimation)) return;

    AnimationData& anim = m_animations[m_currentAnimation];

    int col = anim.currentFrame % anim.cols;
    int row = anim.currentFrame / anim.cols;

    QRect frameRect(col * anim.frameWidth, row * anim.frameHeight,
                    anim.frameWidth, anim.frameHeight);

    QPixmap frame = anim.spriteSheet.copy(frameRect);
    m_currentPixmap = frame.scaled(m_width,m_height,
                                   Qt::IgnoreAspectRatio,
                                   Qt::SmoothTransformation);

    setPixmap(m_currentPixmap);
}

void Pet::startAnimationTimer()
{
    if (!m_animationTimer) {
        m_animationTimer = new QTimer(this);
        connect(m_animationTimer, &QTimer::timeout, this, &Pet::updateAnimationFrame);
    }

    if (m_animationTimer->isActive()) {
        m_animationTimer->stop();
    }

    m_animationTimer->start(1000 / m_animationFps);
}

void Pet::stopAnimationTimer()
{
    if (m_animationTimer && m_animationTimer->isActive()) {
        m_animationTimer->stop();
    }
}

void Pet::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (!m_currentPixmap.isNull()) {
        painter->save();  // 保存状态

        if (m_reverse) {
            // 移动到绘制矩形中心，然后翻转
            painter->translate(0, 0);  //矩形中心在 (0,0)
            painter->scale(-1, 1);
        }

        QRectF drawRect(-m_width / 2.0, -m_height / 2.0, m_width, m_height);
        painter->drawPixmap(drawRect.toRect(), m_currentPixmap);

        painter->restore();  // 恢复状态
    }
}

//宠物技能
void Pet::castLightning(){//释放闪电
    if(!EntityManager::instance()->hasAliveEnemies()) { return; }
    QVector<Entity*> enemies = EntityManager::instance()->getEntitiesByTeam(EntityTeam::ENEMY);
    int size = enemies.size();
    int rd = QRandomGenerator::global()->bounded(0,size);
    QPointF p = enemies[rd]->pos();
    Entity* l = EntityManager::instance()->createEntity<Effect>(p,Effect::LIGHTNING);
    EntityManager::instance()->addToScene(l);
    Effect* lightning = static_cast<Effect*>(l);
    lightning->setCallback([this,p](){
        QVector<Entity*> es = EntityManager::instance()->getEntitiesInRadius(p,100,EntityTeam::ENEMY);
        for(auto e : es){
            if(e && e->isAlive()){
                e->takeDamage(m_attack);
            }
        }
    });
    return;
}


//确定宠物种类
void Pet::catAnimation(){
    m_originalDir = RIGHT;
    m_width = CAT_WIDTH;
    m_height = CAT_HEIGHT;
    m_zoom = CAT_ZOOM;
    m_type = PET_CAT;
    setTransformOriginPoint(m_width / 2, m_height / 2);
    static AnimationData idle("resource/Pets/cat/idle.png",m_width,m_height,4,2,true);
    static AnimationData attack("resource/Pets/cat/idle.png",m_width,m_height,4,2,false);
    static AnimationData walk("resource/Pets/cat/idle.png",m_width,m_height,4,2,false);

    m_animations["idle"] = idle;
    m_animations["attack"] = attack;
    m_animations["walk"] = walk;
}

void Pet::frogAnimation(){
    m_originalDir = LEFT;
    m_width = FROG_WIDTH;
    m_height = FROG_HEIGHT;
    m_zoom = FROG_ZOOM;
    m_type = PET_FROG;
    setTransformOriginPoint(m_width / 2, m_height / 2);
    static AnimationData idle("resource/Pets/frog/idle.png",m_width,m_height,6,6,true);
    static AnimationData attack("resource/Pets/frog/idle.png",m_width,m_height,6,6,false);
    static AnimationData walk("resource/Pets/frog/walk.png",m_width,m_height,5,5,false);

    m_animations["idle"] = idle;
    m_animations["attack"] = attack;
    m_animations["walk"] = walk;
}

void Pet::picaquAnimation(){
    m_originalDir = RIGHT;
    m_width = PICAQU_WIDTH;
    m_height = PICAQU_HEIGHT;
    m_zoom = PICAQU_ZOOM;
    m_type = PET_PICAQU;
    m_skillPointer = &Pet::castLightning;
    setTransformOriginPoint(m_width / 2, m_height / 2);
    static AnimationData idle("resource/Pets/picaqu/idle.png",m_width,m_height,1,1,true);
    static AnimationData attack("resource/Pets/picaqu/idle.png",m_width,m_height,1,1,false);
    static AnimationData walk("resource/Pets/picaqu/walk.png",m_width,m_height,2,2,false);

    m_animations["idle"] = idle;
    m_animations["attack"] = attack;
    m_animations["walk"] = walk;
}
