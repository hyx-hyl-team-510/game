#include "Enemy.h"
#include "Player.h"
#include "EntityManager.h"
#include "audiomanager.h"
#include "CombatSystem.h"
#include "projectile.h"
#include "gamemap.h"
#include <QPainter>
#include <cmath>
#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QPointer>
#include <vector>
CombatSystem* Enemy::combatSystem = nullptr;
int Enemy::BLUE_SLIME_WIDTH = 40;
int Enemy::BLUE_SLIME_HEIGHT = 40;
int Enemy::GREEN_SLIME_WIDTH = 40;
int Enemy::GREEN_SLIME_HEIGHT = 40;
int Enemy::FLYING_MONSTER_WIDTH = 79;
int Enemy::FLYING_MONSTER_HEIGHT = 69;
int Enemy::WIZARD_WIDTH = 64;
int Enemy::WIZARD_HEIGHT = 64;
int Enemy::ASSASSIN_WIDTH = 90;
int Enemy::ASSASSIN_HEIGHT = 90;
int Enemy::SOUL_WIDTH = 160;
int Enemy::SOUL_HEIGHT = 160;
int Enemy::LEFT = -1;
int Enemy::RIGHT = 1;
int Enemy::MELEE_COOLDOWN = 60;
int Enemy::RANGE_COOLDOWN = 180;

qreal Enemy::BLUE_SLIME_ZOOM = 0.5;
qreal Enemy::GREEN_SLIME_ZOOM = 0.5;
qreal Enemy::FLYING_MONSTER_ZOOM = 0.6;
qreal Enemy::WIZARD_ZOOM = 0.6;
qreal Enemy::ASSASSIN_ZOOM = 0.3;
qreal Enemy::SOUL_ZOOM = 0.2;
qreal Enemy::MELEE_ATTACK_RADIUS = 40;
qreal Enemy::RANGE_ATTACK_RADIUS = 350;

Enemy::Enemy(qreal x, qreal y, int level, QGraphicsItem* parent)
    : Entity(EntityType::ENEMY)
    , QGraphicsRectItem(parent)
    , m_level(level)
    , m_attackDirection(1.0, 0.0)
    , m_useAnimation(false)
    , m_animationTimer(nullptr)
    , m_animationFps(12)
    , m_currentDirection("down")
    , m_attackCooldown(0)
{
    // 设置队伍
    setTeam(EntityTeam::ENEMY);

    // 初始化属性
    initEnemyAttributes();

    // 同步 Entity 的生命值
    setMaxHealth(m_maxHealth);
    m_health = m_maxHealth;
    setPos(x, y);

}

Enemy::~Enemy()
{
    stopAnimationTimer();
    qDebug() << "Enemy destroyed";
}

void Enemy::initEnemyAttributes()
{
    // 基础属性
    m_maxHealth = 30 + m_level * 10;
    m_health = m_maxHealth;
    m_damage = 5 + m_level;
    m_moveSpeed = 2.0 + m_level * 0.1;
    m_goldReward = 2;
    // 根据等级设置类型和属性
    if (m_level >= 10 && m_level < 20) {
        m_enemyType = TYPE_ELITE;
        m_maxHealth = static_cast<int>(m_maxHealth * 1.5);
        m_health = m_maxHealth;
        m_damage = static_cast<int>(m_damage * 1.3);
        m_goldReward += 2 * m_level;
    } else if (m_level >= 20) {
        m_enemyType = TYPE_BOSS;
        m_maxHealth = static_cast<int>(m_maxHealth * 10);
        m_health = m_maxHealth;
        m_damage = static_cast<int>(m_damage * 3);
        m_moveSpeed = 1.5;
        m_goldReward += 5 * m_level;
    } else {
        m_enemyType = TYPE_NORMAL;
        m_goldReward += m_level;
    }

    // 同步 Entity 的最大生命值
    setMaxHealth(m_maxHealth);
}
// ========== 重写 Entity 的方法 ==========

void Enemy::update(float deltaTime)
{
    Q_UNUSED(deltaTime);

    if (!isActive() || isDead()) return;

    // 从 EntityManager 获取玩家
    Entity* playerEntity = EntityManager::instance()->getPlayer();
    if (!playerEntity) return;

    Player* player = static_cast<Player*>(playerEntity);

    // 更新冷却
    if (m_attackCooldown > 0) {
        m_attackCooldown--;
    }
    if(m_enemyType == TYPE_BOSS){
        m_bossCooldown--;
    }
    if(m_bossCooldown <= 0){
        bossSkill();
        m_bossCooldown = m_maxBossCooldown;
        m_attackRadius = (qreal)QRandomGenerator::global()->bounded(100,800);
        autoRadius = (qreal)QRandomGenerator::global()->bounded((int)m_attackRadius + 5,1000);
    }
    // 向玩家移动
    if(m_knockbackCnt > 0){
        m_knockbackCnt--;
        setPos(pos() + m_knockbackV);
    }else if(m_currentAnimation != "attack" && m_currentAnimation != "hurt"){
        moveTowards(player->pos(), m_moveSpeed);
    }
    QPointF delta = player->pos() - pos();
    qreal distance = sqrt(delta.x() * delta.x() + delta.y() * delta.y());

    if (distance < m_attackRadius) {
        attack(player);
    }
    avoidOtherEnemies();
}
void Enemy::kill(){
    setActive(false);
    m_health = 0;
    return;
}
void Enemy::takeDamage(int damage, Entity* source)
{
    // 调用基类方法
    Entity::takeDamage(damage, source);
    //播放受伤音效
    AudioManager::instance()->playSound(AudioManager::ENEMY_HURT);
    // 受伤闪烁
    QPointer<Enemy> self(this);
    if(m_animations.contains("hurt")){
        playAnimation("hurt",false);
    }else{
        setEnemyOpacity(0.5);
        QTimer::singleShot(100, [self]() {
            if (!self.isNull()) {
                self->setEnemyOpacity(1.0);
            }
        });
    }
    if (isDead()) {
        Player* player = static_cast<Player*>(EntityManager::instance()->getPlayer());
        player->addGold(m_goldReward);
        if(m_animations.contains("death")){
            playAnimation("death",false);
            QTimer::singleShot(1000,[self](){
                if(!self.isNull()){
                    self->setActive(false);
                    EntityManager::instance()->destroyEntity(self);
                }
            });
        }else{
            setActive(false);
            EntityManager::instance()->destroyEntity(self);
        }
    }
}

// ========== 行为接口 ==========
void Enemy::avoidOtherEnemies()
{
    const qreal RADIUS = m_width * 3;
    const qreal MIN_DISTANCE = m_width * m_zoom;
    const qreal PUSH_FORCE = 0.5;
    Entity* other = EntityManager::instance()->getNearestEnemy(this,RADIUS);
    if(other && other != this && !other->isDead()){
        QPointF delta = pos() - other->pos();
        qreal dist = std::sqrt(delta.x() * delta.x() + delta.y() * delta.y());

        if (dist < MIN_DISTANCE && dist > (qreal)0.01) {
            QPointF pushDir = delta / dist;
            qreal pushDist = MIN_DISTANCE - dist;
            QPointF push = pushDir * pushDist * PUSH_FORCE;
            setPos(pos() + push);
        }
    }
    return;
}

void Enemy::moveTowards(const QPointF& target, qreal speed)
{
    QPointF delta = target - pos();
    qreal length = std::sqrt(delta.x() * delta.x() + delta.y() * delta.y());

    if (length > m_attackRadius - 5 && length < autoRadius) {
        QPointF movement = delta / length * speed;
        QPointF newPos = pos() + movement;
        setPos(newPos);

        // 更新动画方向
        setMoveDirection(movement);
    } else{
        // 没有移动时播放待机动画
        setMoveDirection(QPointF(0, 0));
    }
}

// ========== 攻击接口 ==========

void Enemy::attack(Player* player)
{
    if (m_attackCooldown > 0 || !player) return;
    m_attackCooldown = m_maxAttackCooldown;
    QPointF delta = player->pos() - pos();
    m_reverse = (delta.x() * m_originalDir > 0) ? false : true;
    if(m_attackType == MELEE){//近战
        meleeAttack(player);
    }else{//远程
        rangeAttack(player);
    }
    qDebug() << "Enemy attacks player for" << m_damage << "damage";
}

void Enemy::meleeAttack(Player* player)
{
    if (m_animations.contains("attack")) {
        playAnimation("attack", false);
    }
    player->takeDamage(m_damage);
    return;
}

void Enemy::rangeAttack(Player* player)
{
    if (m_animations.contains("attack")) {
        playAnimation("attack", false);
    }
    QPointF dir = player->pos() - pos();
    QPointer<Enemy> self(this);
    QTimer::singleShot(833,[self,dir](){
        if(!self.isNull()){
            combatSystem->rangeAttack(self,dir);
        }
    });
    return;
}

// ========== 精灵表动画接口 ==========

void Enemy::playAnimation(const QString& name, bool loop)
{
    if (!m_animations.contains(name)) {
        qDebug() << "Enemy: Animation not found:" << name;
        return;
    }

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

void Enemy::stopAnimation()
{
    stopAnimationTimer();
    m_currentAnimation = "";
}

void Enemy::setMoveDirection(const QPointF& velocity)
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
    if (direction == "idle") animationName = "idle";
    else if (direction == "walk") animationName = "walk";

    if (!animationName.isEmpty() && m_animations.contains(animationName)) {
        playAnimation(animationName, true);
    }
}

void Enemy::setEnemyOpacity(qreal opacity)
{
    QGraphicsRectItem::setOpacity(opacity);
}

// ========== 私有方法 ==========

void Enemy::updateAnimationFrame()
{
    if (!isActive() || !m_animations.contains(m_currentAnimation)) return;

    AnimationData& anim = m_animations[m_currentAnimation];
    anim.currentFrame++;

    if (anim.currentFrame >= anim.validFrames) {
        if (anim.loop) {
            anim.currentFrame = 0;
        } else{
            stopAnimation();
            if(isDead()){
                return;
            }
            if (m_currentDirection == "idle" && m_animations.contains("idle")) {
                playAnimation("idle", true);
            }else if (m_currentDirection == "walk" && m_animations.contains("walk")) {
                playAnimation("walk", true);
            }
            return;
        }
    }

    updateCurrentFrame();
}

void Enemy::updateCurrentFrame()
{
    if (!m_animations.contains(m_currentAnimation)) return;

    AnimationData& anim = m_animations[m_currentAnimation];
    int col = anim.currentFrame % anim.cols;
    int row = anim.currentFrame / anim.cols;
    QRect frameRect(col * anim.frameWidth, row * anim.frameHeight,
                    anim.frameWidth, anim.frameHeight);

    QPixmap frame = anim.spriteSheet.copy(frameRect);
    m_currentPixmap = frame.scaled(m_width, m_height,
                                   Qt::IgnoreAspectRatio,
                                   Qt::SmoothTransformation);
    QGraphicsRectItem::update();
}

void Enemy::startAnimationTimer()
{
    if (!m_animationTimer) {
        m_animationTimer = new QTimer(this);
        connect(m_animationTimer, &QTimer::timeout, this, &Enemy::updateAnimationFrame);
    }

    if (m_animationTimer->isActive()) {
        m_animationTimer->stop();
    }

    m_animationTimer->start(1000 / m_animationFps);
}

void Enemy::stopAnimationTimer()
{
    if (m_animationTimer && m_animationTimer->isActive()) {
        m_animationTimer->stop();
    }
}

void Enemy::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
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

void Enemy::knockback(QPointF dir,qreal speed){
    qreal len = std::sqrt(dir.x() * dir.x() + dir.y() * dir.y());
    m_knockbackV = (len > 0.1 ? dir / len * speed : QPointF(0,-speed)) ;
    m_knockbackCnt = m_maxKnockbackCnt;
    return;
}

void Enemy::bossSkill(){
    int choic = QRandomGenerator::global()->bounded(0,4);
    int addX,addY,beginX,beginY,targetX,targetY;
    QPoint curLevel = GameMap::instance()->getCurLevel();
    int i = curLevel.x();
    int j = curLevel.y();
    auto & levelGrid = GameMap::instance()->getLevelGrid();
    QRect rect(levelGrid[i][j][0],levelGrid[i][j][1]);
    switch(choic){
        case 0://向上发射
            addX = 50;
            addY = 0;
            beginX = levelGrid[i][j][0].x();
            beginY = levelGrid[i][j][1].y();
            targetX = 0;
            targetY = -1000;
            break;
        case 1://向下发射
            addX = 50;
            addY = 0;
            beginX = levelGrid[i][j][0].x();
            beginY = levelGrid[i][j][0].y();
            targetX = 0;
            targetY = 1000;
            break;
        case 2://向左发射
            addX = 0;
            addY = 50;
            beginX = levelGrid[i][j][1].x();
            beginY = levelGrid[i][j][0].y();
            targetX = -1000;
            targetY = 0;
            break;
        case 3://向右发射
            addX = 0;
            addY = 50;
            beginX = levelGrid[i][j][0].x();
            beginY = levelGrid[i][j][0].y();
            targetX = 1000;
            targetY = 0;
            break;
    }
    int curX = beginX;
    int curY = beginY;
    while(rect.contains(QPoint(curX,curY))){
        Projectile* projectile = EntityManager::instance()->createEntity<Projectile>(
            QPointF(curX,curY),QPointF(curX + targetX,curY + targetY),m_damage,15.0, 8.0,
            EntityTeam::ENEMY
            );

        EntityManager::instance()->addToScene(projectile);
        projectile->laserIcon();
        curX += addX;
        curY += addY;
    }
    return;
}

void Enemy::blueSlimeAnimation(){
    m_width = BLUE_SLIME_WIDTH;
    m_height = BLUE_SLIME_HEIGHT;
    m_zoom = BLUE_SLIME_ZOOM;
    m_attackType = MELEE;
    m_originalDir = RIGHT;
    m_attackRadius = MELEE_ATTACK_RADIUS;
    m_maxAttackCooldown = MELEE_COOLDOWN;
    setTransformOriginPoint(m_width / 2, m_height / 2);
    static AnimationData idle("resource/Enemy/Mob/blue_slime/blue_slime_walk.png"
    ,80,80,4,4,true);
    static AnimationData walk("resource/Enemy/Mob/blue_slime/blue_slime_walk.png"
    ,80,80,4,4,false);
    static AnimationData attack("resource/Enemy/Mob/blue_slime/blue_slime_attack.png"
    ,80,80,12,6,false);
    m_animations["idle"] = idle;
    m_animations["walk"] = walk;
    m_animations["attack"] = attack;
    m_useAnimation = true;
    playAnimation("idle");
    return;
}

void Enemy::greenSlimeAnimation(){
    m_width = GREEN_SLIME_WIDTH;
    m_height = GREEN_SLIME_HEIGHT;
    m_zoom = GREEN_SLIME_ZOOM;
    m_attackType = MELEE;
    m_originalDir = RIGHT;
    m_attackRadius = MELEE_ATTACK_RADIUS;
    m_maxAttackCooldown = MELEE_COOLDOWN;
    setTransformOriginPoint(m_width / 2, m_height / 2);
    static AnimationData idle("resource/Enemy/Mob/green_slime/green_slime_walk.png"
    ,80,80,4,4,true);
    static AnimationData walk("resource/Enemy/Mob/green_slime/green_slime_walk.png"
    ,80,80,4,4,false);
    static AnimationData attack("resource/Enemy/Mob/green_slime/green_slime_attack.png"
    ,80,80,12,6,false);
    m_animations["idle"] = idle;
    m_animations["walk"] = walk;
    m_animations["attack"] = attack;
    m_useAnimation = true;
    playAnimation("idle");
}

void Enemy::flyingMonsterAnimation(){
    m_width = FLYING_MONSTER_WIDTH;
    m_height = FLYING_MONSTER_HEIGHT;
    m_zoom = FLYING_MONSTER_ZOOM;
    m_attackType = MELEE;
    m_originalDir = LEFT;
    m_attackRadius = MELEE_ATTACK_RADIUS;
    m_maxAttackCooldown = MELEE_COOLDOWN;
    setTransformOriginPoint(m_width / 2, m_height / 2);
    static AnimationData idle("resource/Enemy/Mob/flying_monster/idle.png"
    ,79,69,4,4,true);
    static AnimationData walk("resource/Enemy/Mob/flying_monster/walk.png"
    ,79,69,4,4,false);
    static AnimationData attack("resource/Enemy/Mob/flying_monster/attack.png"
    ,79,69,8,8,false);
    static AnimationData hurt("resource/Enemy/Mob/flying_monster/hurt.png"
    ,79,69,4,4,false);
    static AnimationData death("resource/Enemy/Mob/flying_monster/death.png"
    ,79,69,7,7,false);
    m_animations["idle"] = idle;
    m_animations["walk"] = walk;
    m_animations["attack"] = attack;
    m_animations["hurt"] = hurt;
    m_animations["death"] = death;
    m_useAnimation = true;
    playAnimation("idle");
}

void Enemy::wizardAnimation(){
    m_width = WIZARD_WIDTH;
    m_height = WIZARD_HEIGHT;
    m_zoom = WIZARD_ZOOM;
    m_attackType = RANGE;
    m_originalDir = RIGHT;
    m_attackRadius = RANGE_ATTACK_RADIUS;
    m_maxAttackCooldown = RANGE_COOLDOWN;
    setTransformOriginPoint(m_width / 2, m_height / 2);
    static AnimationData idle("resource/Enemy/Elite/Wizard/idle.png"
    ,64,64,4,4,true);
    static AnimationData walk("resource/Enemy/Elite/Wizard/walk.png"
    ,64,64,8,8,false);
    static AnimationData attack("resource/Enemy/Elite/Wizard/attack.png"
    ,64,64,13,13,false);
    static AnimationData hurt("resource/Enemy/Elite/Wizard/hurt.png"
    ,64,64,3,3,false);
    static AnimationData death("resource/Enemy/Elite/Wizard/death.png"
    ,64,64,11,11,false);
    m_animations["idle"] = idle;
    m_animations["walk"] = walk;
    m_animations["attack"] = attack;
    m_animations["hurt"] = hurt;
    m_animations["death"] = death;
    m_useAnimation = true;
    playAnimation("idle");
}

void Enemy::assassinAnimation(){
    m_width = ASSASSIN_WIDTH;
    m_height = ASSASSIN_HEIGHT;
    m_zoom = ASSASSIN_ZOOM;
    m_attackType = MELEE;
    m_originalDir = RIGHT;
    m_attackRadius = MELEE_ATTACK_RADIUS;
    m_maxAttackCooldown = MELEE_COOLDOWN;
    setTransformOriginPoint(m_width / 2, m_height / 2);
    static AnimationData idle("resource/Enemy/Mob/assassin/assassin_idle.png"
    ,90,90,6,6,true);
    static AnimationData walk("resource/Enemy/Mob/assassin/assassin_walk.png"
    ,90,90,8,8,false);
    static AnimationData attack("resource/Enemy/Mob/assassin/assassin_attack.png"
    ,90,90,7,7,false);
    m_animations["idle"] = idle;
    m_animations["walk"] = walk;
    m_animations["attack"] = attack;
    m_useAnimation = true;
    playAnimation("idle");
}

void Enemy::soulAnimation(){
    m_width = SOUL_WIDTH;
    m_height = SOUL_HEIGHT;
    m_zoom = SOUL_ZOOM;
    m_attackType = RANGE;
    m_originalDir = RIGHT;
    m_attackRadius = RANGE_ATTACK_RADIUS;
    m_maxAttackCooldown = RANGE_COOLDOWN;
    setTransformOriginPoint(m_width / 2, m_height / 2);
    static AnimationData idle("resource/Enemy/Boss/idle.png"
    ,160,160,5,1,true);
    static AnimationData walk("resource/Enemy/Boss/walk.png"
    ,160,160,8,1,false);
    static AnimationData attack("resource/Enemy/Boss/attack.png"
    ,160,160,10,1,false);
    static AnimationData death("resource/Enemy/Boss/death.png"
    ,160,160,8,1,false);
    m_animations["idle"] = idle;
    m_animations["walk"] = walk;
    m_animations["attack"] = attack;
    m_animations["death"] = death;
    m_useAnimation = true;
    playAnimation("idle");
}
