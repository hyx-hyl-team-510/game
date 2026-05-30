#include "Projectile.h"
#include "EntityManager.h"
#include <QPainter>
#include <cmath>
#include <QDebug>
#include <vector>
#include "player.h"
#include "enemy.h"
#include "effect.h"
#include "gamemap.h"
#include "audiomanager.h"
Projectile::Projectile(const QPointF& startPos, const QPointF& targetPos,
                       int damage, qreal radius, qreal speed,
                       EntityTeam team, QGraphicsItem* parent)
    : Entity(EntityType::PROJECTILE)
    , QGraphicsEllipseItem(parent)
    , m_damage(damage)
    , m_speed(speed * 0.5)
    , m_active(true)
    , m_distanceTraveled(0)
    , m_radius(radius)
    , m_angle(0)
{
    setTeam(team);
    setActive(true);

    // 计算方向
    QPointF delta = targetPos - startPos;
    qreal length = std::sqrt(delta.x() * delta.x() + delta.y() * delta.y());
    if (length > 0.1) {
        m_direction = delta / length;
        m_angle = std::atan2(delta.y(), delta.x());
    } else {
        m_direction = QPointF(1.0, 0.0);
    }
    m_pos = startPos;
    setRect(-radius, -radius, radius * 2, radius * 2);
    setPos(startPos);
    setBrush(QBrush(Qt::red));
    setPen(QPen(Qt::black, 1));
}

Projectile::~Projectile()
{
    qDebug() << "Projectile destroyed";
    if(animTimer){
        animTimer->stop();
        delete animTimer;
    }
}
void Projectile::update(float deltaTime)
{
    Q_UNUSED(deltaTime);

    if (!m_active) return;
    if (!isActive()) return;

    // 移动
    m_pos += m_direction * m_speed;
    setPos(QPoint(m_pos.x(),m_pos.y()));
    m_distanceTraveled += m_speed;

    // ========== 碰撞检测 ==========
    //玩家打敌人
    if (getTeam() == EntityTeam::PLAYER) {
        Entity* e = EntityManager::instance()->getNearestEnemy(pos(),m_radius * 3);
        if (e && !e->isDead()) {
            Enemy* enemy = static_cast<Enemy*>(e);
            QPointF delta = pos() - enemy->pos();
            qreal dist = sqrt(delta.x() * delta.x() + delta.y() * delta.y());

            if (dist < m_radius + enemy->getCollisionWidth() / 2) {
                updateEffect();
                enemy->takeDamage(m_damage);
                m_active = false;
                setActive(false);
                EntityManager::instance()->destroyEntity(this);
                qDebug() << "Projectile hit enemy, damage:" << m_damage;
                return;
            }
        }
    }

    //敌人子弹打玩家
    if (getTeam() == EntityTeam::ENEMY) {
        Player* player = static_cast<Player*>(EntityManager::instance()->getPlayer());
        if (player && !player->isDead()) {
            QPointF delta = pos() - player->pos();
            qreal dist = sqrt(delta.x() * delta.x() + delta.y() * delta.y());

            if (dist < m_radius + player->getWidth() / 4) {
                updateEffect();
                player->takeDamage(m_damage);
                m_active = false;
                setActive(false);
                EntityManager::instance()->destroyEntity(this);
                qDebug() << "Enemy projectile hit player, damage:" << m_damage;
                return;
            }
        }
    }
    QPoint curLevel = GameMap::instance()->getCurLevel();
    int i = curLevel.x();
    int j = curLevel.y();
    auto & levelGrid = GameMap::instance()->getLevelGrid();
    QRect rect(levelGrid[i][j][0],levelGrid[i][j][1]);
    QRectF rectf(rect);
    if(!rectf.contains(pos())){//撞墙销毁
        updateEffect();
        m_active = false;
        setActive(false);
        EntityManager::instance()->destroyEntity(this);
    }
}

void Projectile::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->save();  // 保存原始状态

    // 将绘制原点移到图形中心
    painter->translate(rect().center());

    // 旋转（Qt::radiansToDegrees 将弧度转角度）
    painter->rotate(qRadiansToDegrees(m_angle));

    // 绘制图片或圆形（原点在中心，所以从 -radius 开始）
    QRectF drawRect(-m_radius, -m_radius, m_radius * 2, m_radius * 2);
    if(m_useAnimation){
        int col = anim.currentFrame % anim.cols;
        int row = anim.currentFrame / anim.cols;
        QRect frameRect(col * anim.frameWidth, row * anim.frameHeight,
                        anim.frameWidth, anim.frameHeight);
        painter->drawPixmap(drawRect.toRect(),anim.spriteSheet.copy(frameRect));
    }else if (m_usePixmap && !m_pixmap.isNull()) {
        painter->drawPixmap(drawRect.toRect(), m_pixmap);
    }else {
        painter->setBrush(brush());
        painter->setPen(pen());
        painter->drawEllipse(drawRect);
    }

    painter->restore();  // 恢复原始状态
}

void Projectile::updateEffect(){
    if(m_type == FIREBALL){
        Effect* effect = EntityManager::instance()->createEntity<Effect>(pos() - QPointF(64,64),Effect::EXPLOSION);
        EntityManager::instance()->addToScene(effect);
    }
    return;
}

void Projectile::arrowIcon(){
    static QPixmap icon("resource/Projectile/arrow1.png");
    m_usePixmap = true;
    m_pixmap = icon;
    m_angle += M_PI / 4.0;
    m_type = ARROW;
    AudioManager::instance()->playSound(AudioManager::BOW);
    return;
}

void Projectile::fireballIcon(){
    static QPixmap icon("resource/Projectile/fireball.png");
    m_useAnimation = true;
    anim.frameHeight = anim.frameWidth = 32;
    anim.cols = 1;
    anim.validFrames = 4;
    anim.currentFrame = 0;
    anim.spriteSheet = icon;
    m_angle += M_PI / 2 * 3;
    animTimer = new QTimer;
    connect(animTimer,&QTimer::timeout,this,[this](){
        anim.currentFrame = (anim.currentFrame + 1) % anim.validFrames;
    });
    animTimer->start(1000 / 12);
    m_type = FIREBALL;
    AudioManager::instance()->playSound(AudioManager::FIRE);
    return;
}
void Projectile::laserIcon(){
    static QPixmap icon("resource/Projectile/laser.png");
    m_usePixmap = true;
    m_pixmap = icon;
    m_type = LASER;
    return;
}

