#ifndef POTION_H
#define POTION_H
#include "item.h"

class Potion : public Item
{
public:
    enum PotionType{
        MANA_POTION,
        HEALTH_POTION
    };
    Potion(ItemType i_type,QString name,QPixmap& pixmap,PotionType p_type);
    PotionType getType() {return m_type;}
    void onPurchased(Player* player) override;
    ~Potion();
private:
    PotionType m_type;
};

#endif // POTION_H
