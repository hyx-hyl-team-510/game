#ifndef WALL_H
#define WALL_H

#include <QGraphicsPixmapItem>
#include "Entity.h"

class Wall : public Entity, public QGraphicsPixmapItem
{
    Q_OBJECT
    DECLARE_ENTITY_TYPE(Wall, EntityType::WALL)

public:
    Wall(qreal x, qreal y, qreal width, qreal height,bool blank = true, QGraphicsItem* parent = nullptr);

    QGraphicsItem* graphicsItem() override { return this; }
    const QGraphicsItem* graphicsItem() const override { return this; }
};

#endif // WALL_H
