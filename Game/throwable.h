#ifndef THROWABLE_H
#define THROWABLE_H

#include "item.h"

class Player;

class Throwable : public Item
{
public:
    enum ThrowableType{
        FIREBALL
    };
    Throwable(ItemType i_type,QString name,QPixmap& icon,ThrowableType t_type);
    void onPurchased(Player* player) override;
    ThrowableType getType() {return m_type;}
private:
    ThrowableType m_type;
};

#endif // THROWABLE_H
