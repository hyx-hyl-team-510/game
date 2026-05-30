#ifndef GAMEMAP_H
#define GAMEMAP_H
#include <vector>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QGraphicsPixmapItem>
#include "wall.h"
class Enemy;

class GameMap : public QObject
{
    Q_OBJECT
public:
    static GameMap* instance();
    bool startBossLevel(){return m_startBossLevel;}//进入boss关
    bool finishBossLevel(){return m_finishBossLevel;}//结束boss关
    void updateLevel(QPoint p,int w,int h);
    void updateLevel(QPointF p,int w,int h){
        updateLevel(QPoint((int)p.x(),(int)p.y()),w,h);
        return;
    }
    void InitMap(QString path,QPoint p);
    void resetCurLevel();//重置当前关卡
    std::vector<std::vector<std::vector<QPoint>>>& getLevelGrid(){
        return levelGrid;
    }
    QGraphicsPixmapItem* getMap(){return map;}
    QPoint getCurLevel(){return curLevel;}
private:
    GameMap();
    void levelClear();
    void levelReward();//关卡奖励结算
    void updateBgm();//bgm更新
    bool inBossRoom();

    static GameMap* m_gameMap;
    bool m_startBossLevel = false;//进入boss关
    bool m_finishBossLevel = false;//结束boss关
    QGraphicsPixmapItem *map = nullptr;

    QPoint curLevel;
    QPoint leftTop;//地图左上角坐标

    Wall* wall[8];
    bool visit[3][3] = {0};
    bool playDoorSound[3][3] = {0};//标记是否放过关门音效
    std::vector<QPoint> summonPos;//敌人生成点
    std::vector<std::vector<int>> waveData[3][3];//波次数据
    std::vector<std::vector<int>> enemyLevelData[3][3];//敌人等级数据
    std::vector<Wall*> levelWall[3][3];//每个关卡的墙壁
    std::vector<std::vector<std::vector<QPoint>>> levelGrid;//关卡数组，存每一关的左上角点和右下角点
    int waves[3][3] = {0};//波次
    void (Enemy::*enemyPtr[6])();//敌人类型指针
signals:
    void createAIWeapon();
    void createAITalk();
    void movePetToPlayer();
};

#endif // GAMEMAP_H
