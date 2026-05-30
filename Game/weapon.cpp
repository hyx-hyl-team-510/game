#include "Weapon.h"
#include "player.h"

QVector<QPixmap> Weapon::swordIcons;
QVector<QPixmap> Weapon::bowIcons;

int Weapon::swordIcons_cnt = 0;
int Weapon::bowIcons_cnt = 0;

Weapon::Weapon(ItemType i_type,QString name,QPixmap icon,WeaponType w_type,
               int attack,int manaCost,QObject* parent)
    : QObject(parent)
    , Item(i_type,name,icon)
    , m_type(w_type)
    , m_attack(attack)
    , m_manaCost(manaCost)
{

}

Weapon::Weapon(const Weapon& other)
    : QObject(other.parent())
    , Item(other.Item::getType(),other.Item::getName(),other.Item::getIcon())
    , m_type(other.m_type)
    , m_manaCost(other.m_manaCost)
    , m_attack(other.m_attack)
{
}

void Weapon::onPurchased(Player* player){
    player->addWeapon(this);
    return;
}

void Weapon::InitIcons(){
    static bool flag = false;
    if(flag){return;}
    swordIcons.push_back(QPixmap("resource/Weapon/sword_1.png"));
    swordIcons.push_back(QPixmap("resource/Weapon/sword_2.png"));
    swordIcons.push_back(QPixmap("resource/Weapon/sword_3.png"));
    swordIcons.push_back(QPixmap("resource/Weapon/sword_4.png"));

    bowIcons.push_back(QPixmap("resource/Weapon/bow_1.png"));
    bowIcons.push_back(QPixmap("resource/Weapon/bow_2.png"));
    bowIcons.push_back(QPixmap("resource/Weapon/bow_3.png"));
    bowIcons.push_back(QPixmap("resource/Weapon/bow_4.png"));
    flag = true;
    return;
}

QPixmap& Weapon::baseSwordIcon(){
    static QPixmap icon("resource/Weapon/base_sword.png");
    return icon;
}

QPixmap& Weapon::baseBowIcon(){
    static QPixmap icon("resource/Weapon/base_bow.png");
    return icon;
}

QPixmap& Weapon::getSwordIcon(){
    swordIcons_cnt %= static_cast<int>(swordIcons.size());
    return swordIcons[swordIcons_cnt++];
}

QPixmap& Weapon::getBowIcon(){
    bowIcons_cnt %= static_cast<int>(bowIcons.size());
    return bowIcons[bowIcons_cnt++];
}

