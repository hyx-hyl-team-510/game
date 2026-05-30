#ifndef ITEM_H
#define ITEM_H
#include <QPixmap>
#include <QString>
class Player;
struct ItemData{
    int health_potion_count;
    int mana_potion_count;
    int fireball_count;
    ItemData(){
        health_potion_count = 0;
        mana_potion_count = 0;
        fireball_count = 0;
    }
};

class Item
{
public:
    enum ItemType{
        NONE = -1,
        ARMOR,
        ACCESSORY,
        WEAPON,
        POTION,
        THROWABLE,
        GOLD
    };
    Item(ItemType type,QString name,QPixmap icon);
    ~Item();
    virtual void onPurchased(Player* player);//执行购买后效果的函数

    ItemType getType() {return m_type;}
    QString& getName() {return m_name;}
    QPixmap& getIcon() {return m_icon;}
    ItemType getType() const {return m_type;}
    QString getName() const {return m_name;}
    QPixmap getIcon() const {return m_icon;}
    void setIcon(QPixmap& icon) {m_icon = icon;}
    static const int HEALTH_POTION_EFFECT;//生命药水效果
    static const int MANA_POTION_EFFECT;//魔法药水效果
    static const int FIREBALL_ATTACK;//火球伤害
    static QPixmap& healthPotionIcon();//获取生命药水图片
    static QPixmap& manaPotionIcon();//获取魔法药水图片
    static QPixmap& fireballIcon();//获取火球图片
    static QPixmap& goldIcon();//获取金币图片
private:
    ItemType m_type;
    QPixmap m_icon;
    QString m_name;
};

#endif // ITEM_H
