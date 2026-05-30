#include "throwable.h"
#include "Player.h"

Throwable::Throwable(ItemType i_type,QString name,QPixmap& icon,ThrowableType t_type)
    : Item(i_type,name,icon)
    , m_type(t_type)
{

}

void Throwable::onPurchased(Player* player){
    switch(m_type){
    case FIREBALL:
        player->getItems().fireball_count++;
        break;
    }
    return;
}

