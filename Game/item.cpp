#include "item.h"
const int Item::HEALTH_POTION_EFFECT = 100;
const int Item::MANA_POTION_EFFECT = 50;
const int Item::FIREBALL_ATTACK = 20;
Item::Item(ItemType type,QString name,QPixmap icon)
    : m_type(type)
    , m_name(name)
    , m_icon(icon)
{

}


Item::~Item(){}

void Item::onPurchased(Player* player){
    return;
}

QPixmap& Item::healthPotionIcon(){
    static QPixmap icon("resource/Item/health_potion.png");
    return icon;
}

QPixmap& Item::manaPotionIcon(){
    static QPixmap icon(("resource/Item/mana_potion.png"));
    return icon;
}

QPixmap& Item::fireballIcon(){
    static QPixmap icon("resource/Item/fireball.png");
    return icon;
}
QPixmap& Item::goldIcon(){
    static QPixmap icon("resource/Bar/gold.png");
    return icon;
}
