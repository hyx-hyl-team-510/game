#ifndef WEAPON_H
#define WEAPON_H

#include "item.h"
#include <QObject>
#include <QString>
#include <QPixmap>
#include <QVector>
class Player;
class Weapon : public QObject,public Item
{
    Q_OBJECT

public:
    enum WeaponType {
        SWORD,      // 剑（近战）
        BOW,        // 弓（远程）
        STAFF       // 法杖（魔法）
    };
    Weapon(ItemType i_type,QString name,QPixmap icon,WeaponType w_type,
           int attack,int manaCost,QObject* parent = nullptr);

    // 拷贝构造函数
    Weapon(const Weapon& other);

    // 基础属性
    int getAttack() const { return m_attack; }
    int getManaCost() const{return m_manaCost;}
    WeaponType getType() {return m_type;}
    void onPurchased(Player* player) override;
    static QPixmap& baseSwordIcon();
    static QPixmap& baseBowIcon();

    static QPixmap& getSwordIcon();
    static QPixmap& getBowIcon();
    static void InitIcons();
private:
    WeaponType m_type;//武器类型
    int m_attack;
    int m_manaCost;

    static QVector<QPixmap> swordIcons;
    static QVector<QPixmap> bowIcons;

    static int swordIcons_cnt;
    static int bowIcons_cnt;


};

#endif // WEAPON_H
