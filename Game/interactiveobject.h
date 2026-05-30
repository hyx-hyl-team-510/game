#ifndef INTERACTIVEOBJECT_H
#define INTERACTIVEOBJECT_H
#include <QGraphicsPixmapItem>
#include "Entity.h"


class InteractiveObject : public Entity,public QGraphicsPixmapItem
{
    Q_OBJECT
    DECLARE_ENTITY_TYPE(Wall, EntityType::WALL)
public:
    InteractiveObject(qreal x,qreal y,qreal r,int w,int h,QGraphicsItem* parent = nullptr);
    QGraphicsItem* graphicsItem() override { return this; }
    const QGraphicsItem* graphicsItem() const override { return this; }
    void setStaticPixmap(QString path);
    void setStaticPixmap(QPixmap& pixmap);
    bool isInteractive(QPointF p);//是否可以交互
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
private:
    int m_width;
    int m_height;
    qreal m_interactiveRadius;
    QPixmap m_pixmap;

};

#endif // INTERACTIVEOBJECT_H
