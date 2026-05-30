#include "Wall.h"

Wall::Wall(qreal x, qreal y, qreal width, qreal height,bool blank, QGraphicsItem* parent)
    : Entity(EntityType::WALL)
    , QGraphicsPixmapItem(parent)
{
    Entity::setPos(x, y);
    setTeam(EntityTeam::NEUTRAL);
    if(blank){
        QPixmap pix(width,height);
        pix.fill(Qt::transparent);
        setPixmap(pix);
    }
}
