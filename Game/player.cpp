#include "Player.h"
#include "EntityManager.h"
#include "audiomanager.h"
#include "Weapon.h"
#include "effect.h"
#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QPointer>
#include <cmath>
#include "enemy.h"

// 初始化静态运动参数
qreal Player::MAX_SPEED = 5.0;
qreal Player::ACCELERATION = 0.5;
qreal Player::FRICTION = 0.92;

Player::Player(qreal x, qreal y, QGraphicsItem* parent)
    : Entity(EntityType::PLAYER)
    , QGraphicsPixmapItem(parent)
    , m_mana(200)
    , m_maxMana(200)
    , m_moveSpeed(5.0)
    , m_velocity(0, 0)
    , m_lastMoveDirection(1.0, 0.0)
    , m_keyUp(false), m_keyDown(false), m_keyLeft(false), m_keyRight(false)
    , m_useAnimation(false)
    , m_animationTimer(nullptr)
    , m_animationFps(12)
    , m_currentWeapon(nullptr)
    , m_weaponCapacity(2)
    , m_maxWeaponCapacity(6)
    , m_baseAttack(5)
    , m_gold(99999)
    , m_anotherPos(3000,3000)
{
    m_width = PLAYER_WIDTH;
    m_height = PLAYER_HEIGHT;
    m_zoom = 1.0;
    setPos(x, y);
    setTransformOriginPoint(m_width / 2, m_height / 2);
    setFlag(QGraphicsItem::ItemIsFocusable);

    // 设置 Entity 属性
    setTeam(EntityTeam::PLAYER);
    initPlayerAttributes();
}

Player::~Player()
{
    stopAnimationTimer();

    if (m_currentWeapon) {
        delete m_currentWeapon;
        m_currentWeapon = nullptr;
    }
    for (Weapon* w : m_ownedWeapons) {
        delete w;
    }
    m_ownedWeapons.clear();
}

void Player::initPlayerAttributes()
{
    m_maxHealth = 1000;
    m_health = m_maxHealth;
    setMaxHealth(m_maxHealth);
}

// ========== 重写 Entity 的方法 ==========

void Player::update(float deltaTime)
{
    Q_UNUSED(deltaTime);

    if (!isActive() || isDead()) return;
    if(m_skillCoolDown > 0){
        m_skillCoolDown--;
    }
    // 更新移动
    updateMovement();
    updateAutoAim();
    /*if(m_animations.contains(m_currentAnimation) && m_currentAnimation.count("attack")){
        isAttack = true;
    }else{
        isAttack = false;
    }*/
    // 应用速度移动位置
    QPointF newPos = pos() + m_velocity;
    setPos(newPos);
}

void Player::takeDamage(int damage, Entity* source)
{
    Q_UNUSED(source);
    if(isDead()){return;}
    // 调用基类方法
    Entity::takeDamage(damage, nullptr);
    //播放受伤音效
    AudioManager::instance()->playSound(AudioManager::CHARACTER_HURT);
    setPlayerOpacity(0.5);
    QPointer<Player> self(this);
    QTimer::singleShot(100, [self]() {
        if (!self.isNull()) {
            self->setPlayerOpacity(1.0);
        }
    });
    if(isDead()){
        playAnimation("death",true);
        AudioManager::instance()->playSound(AudioManager::CHARACTER_DEATH);
    }

    qDebug() << "Player took" << damage << "damage, health:" << m_health;
}

// ========== 移动接口 ==========

void Player::setKeyState(int key, bool pressed)
{
    if (key == Qt::Key_W || key == Qt::Key_Up) m_keyUp = pressed;
    if (key == Qt::Key_S || key == Qt::Key_Down) m_keyDown = pressed;
    if (key == Qt::Key_A || key == Qt::Key_Left) m_keyLeft = pressed;
    if (key == Qt::Key_D || key == Qt::Key_Right) m_keyRight = pressed;
}

void Player::updateMovement()
{
    QPointF acceleration(0, 0);
    if (m_keyUp) acceleration.setY(-ACCELERATION);
    if (m_keyDown) acceleration.setY(ACCELERATION);
    if (m_keyLeft) acceleration.setX(-ACCELERATION);
    if (m_keyRight) acceleration.setX(ACCELERATION);

    m_velocity += acceleration;

    qreal speed = sqrt(m_velocity.x() * m_velocity.x() + m_velocity.y() * m_velocity.y());
    if (speed > MAX_SPEED) {
        m_velocity = m_velocity / speed * MAX_SPEED;
    }

    if (acceleration.x() == 0 && acceleration.y() == 0) {
        m_velocity *= 0;
    }
    if (m_velocity.x() != 0 || m_velocity.y() != 0) {
        m_lastMoveDirection = m_velocity;
    }
    qreal currentSpeed = sqrt(m_velocity.x() * m_velocity.x() + m_velocity.y() * m_velocity.y());
    if (m_velocity.x() > 0.5) {
        isLeft = false;
        isRight = true;
        if(m_attackDirection.x() < 0){
            isLeft = true;
            isRight = false;
        }else{
            isLeft = false;
            isRight = true;
        }
        setMoveDirection("right");
    } else if (m_velocity.x() < -0.5) {
        isLeft = true;
        isRight = false;
        if(m_attackDirection.x() < 0){
            isLeft = true;
            isRight = false;
        }else{
            isLeft = false;
            isRight = true;
        }
        setMoveDirection("left");
    } else if (m_velocity.y() > 0.5) {
        setMoveDirection("down");
    } else if (m_velocity.y() < -0.5) {
        setMoveDirection("up");
    } else if (currentSpeed < 0.5) {
        setMoveDirection("idle");
    }
}

void Player::setPosition(const QPointF& pos)
{
    setPos(pos);
}

// ========== 攻击接口 ==========

void Player::attack()
{   if(isAttack){return;}
    int manaCost = baseManaCost;
    if(m_currentWeapon->getType() == Weapon::SWORD){//用剑
        manaCost += m_currentWeapon->getManaCost();
        if(manaCost > m_mana){
            return;
        }
        m_mana -= manaCost;
        AudioManager::instance()->playSound(AudioManager::SWORD_ATTACK);
        if (m_animations.contains("attack_sword")) {
            playAnimation("attack_sword", false);
        }
    }else if(m_currentWeapon->getType() == Weapon::BOW){//用弓
        manaCost += m_currentWeapon->getManaCost();
        if(manaCost > m_mana){
            return;
        }
        m_mana -= manaCost;
        if (m_animations.contains("attack_bow")) {
            playAnimation("attack_bow", false);
        }
    }
    isAttack = true;
    return;
}

// ========== 属性接口 ==========

void Player::setHealth(int health)
{
    m_health = qBound(0, health, m_maxHealth);
}

void Player::setMana(int mana)
{
    m_mana = qBound(0, mana, m_maxMana);
}

void Player::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (!m_currentPixmap.isNull()) {
        painter->save();
        if(isLeft){
            painter->translate(0,0);
            painter->scale(-1, 1);
        }
        QRect rect(-m_width / 2,-m_height / 2,m_width,m_height);
        painter->drawPixmap(rect,m_currentPixmap);
        painter->restore();
    }
}

// ========== 精灵表动画接口 ==========

void Player::loadAnimation(const QString& name, const QString& imagePath,
                           int frameWidth, int frameHeight, int validFrameCount)
{
    QPixmap spriteSheet;
    if (!spriteSheet.load(imagePath)) {
        qDebug() << "Failed to load animation:" << name;
        return;
    }
    AnimationData anim;
    anim.spriteSheet = spriteSheet;
    anim.frameWidth = frameWidth;
    anim.frameHeight = frameHeight;
    anim.currentFrame = 0;
    anim.loop = true;
    anim.cols = spriteSheet.width() / frameWidth;
    anim.validFrames = qMin(validFrameCount, anim.cols * (spriteSheet.height() / frameHeight));
    m_animations[name] = anim;
    m_useAnimation = true;
}

void Player::playAnimation(const QString& name, bool loop)
{
    if (!m_animations.contains(name)) {
        qDebug() << "Animation not found:" << name;
        return;
    }

    if (m_currentAnimation == name) return;

    AnimationData& anim = m_animations[name];
    anim.currentFrame = 0;
    anim.loop = loop;
    m_currentAnimation = name;

    updateCurrentFrame();
    startAnimationTimer();
}

void Player::stopAnimation()
{
    stopAnimationTimer();
    m_currentAnimation = "";
}

void Player::setMoveDirection(const QString& direction)
{
    if (m_currentDirection == direction) return;
    m_currentDirection = direction;
    if(isAttack){
        return;
    }
    QString animationName;

    if (direction == "idle"){
        animationName = getWeaponIdleName();
    }else{
        animationName = getWeaponWalkName(direction);
    }
    if (!animationName.isEmpty() && m_animations.contains(animationName)) {
        playAnimation(animationName, true);
    } else if (m_animations.contains("idle")) {
        playAnimation("idle", true);
    }
}

void Player::setPlayerOpacity(qreal opacity)
{
    QGraphicsPixmapItem::setOpacity(opacity);
}

// ========== 私有动画方法 ==========

void Player::updateAnimationFrame()
{
    if (!m_animations.contains(m_currentAnimation)) return;
    AnimationData& anim = m_animations[m_currentAnimation];
    anim.currentFrame++;
    if(isAttack && anim.currentFrame == anim.validFrames / 2){
        emit performAttack();
    }
    if (anim.currentFrame >= anim.validFrames) {
        if (anim.loop) {
            anim.currentFrame = 0;
        } else {
            stopAnimation();
            m_currentDirection = "none";
            isAttack = false;
            return;
        }
    }

    updateCurrentFrame();
}

void Player::updateCurrentFrame()
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

void Player::startAnimationTimer()
{
    if (!m_animationTimer) {
        m_animationTimer = new QTimer(this);
        connect(m_animationTimer, &QTimer::timeout, this, &Player::updateAnimationFrame);
    }

    if (m_animationTimer->isActive()) {
        m_animationTimer->stop();
    }

    m_animationTimer->start(1000 / m_animationFps);
}

void Player::stopAnimationTimer()
{
    if (m_animationTimer && m_animationTimer->isActive()) {
        m_animationTimer->stop();
    }
}

// ========== 武器背包系统 ==========

void Player::setWeaponCapacity(int capacity)
{
    int newCapacity = qBound(1, capacity, m_maxWeaponCapacity);
    if (m_weaponCapacity == newCapacity) return;

    m_weaponCapacity = newCapacity;

    while (m_ownedWeapons.size() > m_weaponCapacity) {
        Weapon* removed = m_ownedWeapons.takeLast();
        qDebug() << "Weapon removed due to capacity reduction:" << removed->getName();
        delete removed;
    }

    emit weaponCapacityUpgraded(m_weaponCapacity);
    emit weaponInventoryChanged();
}

void Player::upgradeWeaponCapacity(int additionalSlots)
{
    setWeaponCapacity(m_weaponCapacity + additionalSlots);
    qDebug() << "Weapon capacity upgraded to:" << m_weaponCapacity;
}

int Player::getOwnedWeaponCount() const
{
    return m_ownedWeapons.size();
}

bool Player::isWeaponInventoryFull() const
{
    return m_ownedWeapons.size() >= m_weaponCapacity;
}

bool Player::addWeapon(Weapon* weapon)
{
    if (!weapon) return false;
    if(!m_currentWeapon){
        m_currentWeapon = weapon;
        return true;
    }
    if (isWeaponInventoryFull()) {
        qDebug() << "Weapon inventory full! Cannot add:" << weapon->getName();
        return false;
    }
    m_ownedWeapons.append(weapon);
    qDebug() << "Weapon added:" << weapon->getName()
             << "(" << m_ownedWeapons.size() << "/" << m_weaponCapacity << ")";

    emit weaponInventoryChanged();
    return true;
}

bool Player::removeWeapon()//丢掉当前武器
{
    if(!m_currentWeapon || m_ownedWeapons.empty()){
        return false;
    }
    m_currentWeapon->deleteLater();
    m_currentWeapon = nullptr;
    switchToNextWeapon();
    return true;
}

void Player::equipWeapon(Weapon* weapon)
{
    if (!weapon) return;

    for (int i = 0; i < m_ownedWeapons.size(); ++i) {
        if (m_ownedWeapons[i] == weapon) {
            m_ownedWeapons.removeAt(i);
            break;
        }
    }
    m_currentWeapon = weapon;
    switchWeaponSound();
    isAttack = false;//换武器时停止攻击
    if (m_currentDirection == "idle") {//换武器动画
        playAnimation(getWeaponIdleName(), true);
    }else{
        m_currentDirection = "none";//更新移动动画
    }
    qDebug() << "Equipped weapon:" << weapon->getName()
             << "Attack:" << weapon->getAttack();

    emit weaponEquipped(weapon);
    emit weaponInventoryChanged();
}

void Player::unequipWeapon()
{
    if (!m_currentWeapon) return;

    if (isWeaponInventoryFull()) {
        qDebug() << "Cannot unequip: inventory full";
        return;
    }

    m_ownedWeapons.append(m_currentWeapon);
    m_currentWeapon = nullptr;

    qDebug() << "Weapon unequipped";

    emit weaponUnequipped();
    emit weaponInventoryChanged();
}

bool Player::switchToNextWeapon()
{
    if (m_ownedWeapons.isEmpty()) return false;
    if(m_currentWeapon){
        m_ownedWeapons.append(m_currentWeapon);
        m_currentWeapon = nullptr;
    }
    equipWeapon(m_ownedWeapons[0]);
    return true;
}

int Player::getTotalAttack() const
{
    int total = m_baseAttack;
    if (m_currentWeapon) {
        total += m_currentWeapon->getAttack();
    }
    return total;
}

QString Player::getWeaponIdleName() const
{
    if (!m_currentWeapon) return "idle";

    switch (m_currentWeapon->getType()) {
    case Weapon::BOW:   return "idle_bow";
    case Weapon::STAFF: return "idle_staff";
    case Weapon::SWORD:
    default:            return "idle_sword";
    }
}
QString Player::getWeaponWalkName(const QString& direction){
    QString type,animationName;
    if(!m_currentWeapon){
        type = "";
    }else{
        switch (m_currentWeapon->getType()) {
        case Weapon::BOW:   type = "_bow";break;
        case Weapon::STAFF: type = "_staff";break;
        case Weapon::SWORD:
        default:            type = "_sword";break;
        }
    }
    animationName = "walk_" + direction + type;
    return animationName;
}
void Player::setAttackDirection(QPointF dir){
    m_attackDirection = dir;
    return;
}
void Player::setTarget(Enemy* target){
    m_target = target;
    return;
}

QPointF Player::getPlayerAttackDirection(){
    // 优先使用玩家移动方向
    if (m_velocity.x() != 0 || m_velocity.y() != 0) {
        return m_velocity;
    }else{
        return m_lastMoveDirection;
    }
}
void Player::updateAutoAim(){
    if (m_target && m_target->isAlive()) {
        // 指向目标敌人
        QPointF startPos = this->pos();
        m_attackDirection = m_target->pos() - startPos;
    } else {
        // 没有敌人在范围内，使用移动方向
        m_attackDirection = getPlayerAttackDirection();
    }
    return;
}

void Player::timeStopAttack(){
    if(m_currentWeapon->getType() == Weapon::SWORD){//用剑
        AudioManager::instance()->playSound(AudioManager::SWORD_ATTACK);
        if (m_animations.contains("attack_sword")) {
            playAnimation("attack_sword", false);
        }
        Entity* slash = EntityManager::instance()->createEntity<Effect>(pos(),Effect::SLASH);
        EntityManager::instance()->addToScene(slash);
    }else if(m_currentWeapon->getType() == Weapon::BOW){//用弓
        if (m_animations.contains("attack_bow")) {
            playAnimation("attack_bow", false);
        }
    }
    emit performAttack();
    return;
}

void Player::switchWeaponSound(){
    if(!m_currentWeapon){return;}
    if(m_currentWeapon->getType() == Weapon::SWORD){
        AudioManager::instance()->playSound(AudioManager::SWORD_DRAW);
    }
    return;
}
