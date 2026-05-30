#include "potion.h"
#include "player.h"
Potion::Potion(ItemType i_type,QString name,QPixmap& pixmap,PotionType p_type)
    : Item(i_type,name,pixmap)
    , m_type(p_type)
{

}

void Potion::onPurchased(Player* player){
    if(!player || player->isDead()){
        return;
    }
    switch(m_type){
    case HEALTH_POTION:
        player->getItems().health_potion_count++;
        break;
    case MANA_POTION:
        player->getItems().mana_potion_count++;
        break;
    }
    return;
}

Potion::~Potion(){

}
