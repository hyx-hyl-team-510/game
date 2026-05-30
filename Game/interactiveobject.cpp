#include "interactiveobject.h"
#include <QPainter>
#include <cmath>

InteractiveObject::InteractiveObject(qreal x,qreal y,qreal r,int w,int h,QGraphicsItem* parent):
    Entity(EntityType::INTERACTIVE_OBJECT),QGraphicsPixmapItem(parent)
{
    Entity::setPos(x,y);
    Entity::setVisible(true);
    setTeam(EntityTeam::NEUTRAL);
    m_width = w;
    m_height = h;
    m_interactiveRadius = r;
}

void InteractiveObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->save();
    painter->translate(0,0);
    QRect rect(-m_width / 2,-m_height / 2,m_width,m_height);
    painter->drawPixmap(rect,m_pixmap);
    painter->restore();
    return;
}

void InteractiveObject::setStaticPixmap(QString path){
    m_pixmap.load(path);
    setPixmap(m_pixmap);
    return;
}
void InteractiveObject::setStaticPixmap(QPixmap& pixmap){
    m_pixmap = pixmap;
    setPixmap(m_pixmap);
    return;
}

bool InteractiveObject::isInteractive(QPointF p){
    QPointF dir = Entity::pos() - p;
    qreal dist = std::sqrt(dir.x() * dir.x() + dir.y() * dir.y());
    return dist <= m_interactiveRadius;
}
