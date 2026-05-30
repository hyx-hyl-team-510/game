#include "gamemap.h"
#include "entitymanager.h"
#include "audiomanager.h"
#include "enemy.h"
#include "interactiveobject.h"
#include <QDebug>
#include <QRandomGenerator>

GameMap* GameMap::m_gameMap = nullptr;

GameMap* GameMap::instance(){
    if(!m_gameMap){
        m_gameMap = new GameMap;
    }
    return m_gameMap;
}

GameMap::GameMap() {}

void GameMap::levelClear(){
    int i = curLevel.x();
    int j = curLevel.y();
    if(visit[i][j]){return;}
    AudioManager::instance()->playSound(AudioManager::VICTORY);
    visit[i][j] = true;
    for(Wall* wall : levelWall[i][j]){
        wall->Entity::setVisible(false);
    }
    if(inBossRoom() && m_startBossLevel && !m_finishBossLevel){
        m_startBossLevel = false;
        m_finishBossLevel = true;
    }
    levelReward();
    emit createAITalk();
    return;
}

void GameMap::levelReward(){
    int i = curLevel.x() * 3 + curLevel.y();
    switch(i){
        case 0:
        {
            InteractiveObject* shopItem = new InteractiveObject(500,500,100,160,96);
            shopItem->setZValue(-2);
            shopItem->setStaticPixmap("resource/Decoration/shop2.png");
            EntityManager::instance()->setShopItem(shopItem);
            EntityManager::instance()->addToScene(shopItem);
            break;
        }
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            emit createAIWeapon();
            break;
    }
}

void GameMap::resetCurLevel(){
    int i = curLevel.x();
    int j = curLevel.y();
    if(!visit[i][j]){//未通过该关卡
        waves[i][j] = waves[i][j] == 0 ? 0 : waves[i][j] - 1;
    }
    return;
}

void GameMap::updateLevel(QPoint p,int w,int h){
    updateBgm();
    QPoint p1 = p - QPoint(w / 2,h / 2);
    QPoint p2 = p + QPoint(w / 2,h / 2);
    for(int i=0;i<3;i++){
        for(int j=0;j<3;j++){
            QPoint begin = levelGrid[i][j][0];
            QPoint end = levelGrid[i][j][1];
            QRect rect(begin,end);
            if(rect.contains(p1) && rect.contains(p2)){
                curLevel = QPoint(i,j);
                break;
            }
        }
    }
    int i = curLevel.x();
    int j = curLevel.y();
    if(visit[i][j] == false){
        for(Wall* wall : levelWall[i][j]){
            wall->Entity::setVisible(true);
        }
    }
    if(EntityManager::instance()->getEnemyCount() == 0){//更新波次
        if(waves[i][j] < (int)waveData[i][j].size()){
            int wave = waves[i][j]++;
            int size = waveData[i][j][wave].size();
            for(int k=0;k<size;k++){
                int cnt = waveData[i][j][wave][k];
                int level = enemyLevelData[i][j][wave][k];
                int r_index = QRandomGenerator::global()->bounded(0,(int)summonPos.size());
                QPoint p = levelGrid[i][j][0] + summonPos[r_index];
                if(k == 5){p = levelGrid[i][j][0] + QPoint(500,500);}//boss位于关卡中心
                while(cnt--){
                    Enemy *enemy = EntityManager::instance()->createEntity<Enemy>(p.x(),p.y(),level);
                    (enemy->*enemyPtr[k])();
                    EntityManager::instance()->addToScene(enemy);
                }
            }
        }else{
            levelClear();
        }
    }
    return;
}

void GameMap::InitMap(QString path,QPoint p){
    leftTop = p;
    map = new QGraphicsPixmapItem(QPixmap(path));
    map->setPos(p);
    map->setZValue(-3);
    curLevel = QPoint(0,0);
    wall[0] = EntityManager::instance()->createEntity<Wall>(450,1050,200,50,false);
    wall[1] = EntityManager::instance()->createEntity<Wall>(450,2100,200,50,false);
    wall[2] = EntityManager::instance()->createEntity<Wall>(1050,2550,50,200,false);
    wall[3] = EntityManager::instance()->createEntity<Wall>(1500,2100,200,50,false);
    wall[4] = EntityManager::instance()->createEntity<Wall>(1500,1050,200,50,false);
    wall[5] = EntityManager::instance()->createEntity<Wall>(2100,450,50,200,false);
    wall[6] = EntityManager::instance()->createEntity<Wall>(2550,1050,200,50,false);
    wall[7] = EntityManager::instance()->createEntity<Wall>(2550,2100,200,50,false);
    wall[2]->setPixmap(QPixmap("resource/Map/wall2.png"));
    wall[5]->setPixmap(QPixmap("resource/Map/wall2.png"));
    for(int i=0;i<8;i++){
        if(i != 2 && i != 5){
            wall[i]->setPixmap(QPixmap("resource/Map/wall1.png"));
        }
        EntityManager::instance()->addToScene(wall[i]);
    }
    levelGrid = std::vector<std::vector<std::vector<QPoint>>>(3,std::vector<std::vector<QPoint>>(3,std::vector<QPoint>(2)));
    levelGrid[0][0][0] = QPoint(50,50);//关卡边界
    levelGrid[0][0][1] = QPoint(1049,1049);
    levelGrid[0][1][0] = QPoint(1100,50);
    levelGrid[0][1][1] = QPoint(2099,1049);
    levelGrid[0][2][0] = QPoint(2150,50);
    levelGrid[0][2][1] = QPoint(3149,1049);

    levelGrid[1][0][0] = QPoint(50,1100);
    levelGrid[1][0][1] = QPoint(1049,2099);
    levelGrid[1][1][0] = QPoint(1100,1100);
    levelGrid[1][1][1] = QPoint(2099,2099);
    levelGrid[1][2][0] = QPoint(2150,1100);
    levelGrid[1][2][1] = QPoint(3149,2099);

    levelGrid[2][0][0] = QPoint(50,2150);
    levelGrid[2][0][1] = QPoint(1049,3149);
    levelGrid[2][1][0] = QPoint(1100,2150);
    levelGrid[2][1][1] = QPoint(2099,3149);
    levelGrid[2][2][0] = QPoint(2150,2150);
    levelGrid[2][2][1] = QPoint(3149,3149);

    levelWall[0][0] = {wall[0]};//墙壁绑定
    levelWall[0][1] = {wall[4],wall[5]};
    levelWall[0][2] = {wall[5],wall[6]};
    levelWall[1][0] = {wall[0],wall[1]};
    levelWall[1][1] = {wall[3],wall[4]};
    levelWall[1][2] = {wall[6],wall[7]};
    levelWall[2][0] = {wall[1],wall[2]};
    levelWall[2][1] = {wall[2],wall[3]};
    levelWall[2][2] = {wall[7]};
    //设置每关波次,每波各种敌人各多少个
    waveData[0][0] = {{5},{6,3},{0,0,2}};  //1.1
    waveData[0][1] = {{5,3},{0,0,2},{0,0,3,0,6},{0,0,3,2,6},{0,0,5,5,7},{0,0,5,5,7}};  //2.4
    waveData[0][2] = {{5,3},{0,0,2},{0,0,3,0,6},{0,0,3,2,4},{0,0,5,4,6}};  //2.3
    waveData[1][0] = {{5,3},{0,0,3},{0,0,3,0,6},{0,0,4,0,8},{0,0,5,1,8}};  //1.2
    waveData[1][1] = {{0,0,0,0,0,1}};  //boss
    waveData[1][2] = {{5,3},{0,0,2},{0,0,2,0,4},{0,0,3,2,6}};  //2.2
    waveData[2][0] = {{5,3},{0,0,3},{0,0,3,0,6},{0,0,4,2,8},{0,0,5,4,10},{0,0,6,5,10}};  //1.3
    waveData[2][1] = {{5,3},{0,0,3},{0,0,3,0,6},{0,0,4,2,8},{0,0,6,5,10},{0,0,6,6,10},{0,0,6,6,10},{0,0,6,6,10}};  //1.4
    waveData[2][2] = {{5},{6,3}};
    //敌人的相对生成位置
    summonPos.push_back(QPoint(100,100));
    summonPos.push_back(QPoint(200,100));
    summonPos.push_back(QPoint(300,100));
    summonPos.push_back(QPoint(800,100));
    summonPos.push_back(QPoint(100,200));
    summonPos.push_back(QPoint(200,300));
    summonPos.push_back(QPoint(700,800));
    summonPos.push_back(QPoint(500,700));
    summonPos.push_back(QPoint(500,200));
    summonPos.push_back(QPoint(600,700));
    summonPos.push_back(QPoint(600,400));
    summonPos.push_back(QPoint(600,300));
    summonPos.push_back(QPoint(800,200));
    summonPos.push_back(QPoint(800,400));
    summonPos.push_back(QPoint(800,700));
    summonPos.push_back(QPoint(800,800));
    summonPos.push_back(QPoint(800,300));
    summonPos.push_back(QPoint(400,300));
    summonPos.push_back(QPoint(400,800));
    summonPos.push_back(QPoint(400,500));
    summonPos.push_back(QPoint(400,120));
    summonPos.push_back(QPoint(400,350));
    summonPos.push_back(QPoint(350,450));
    summonPos.push_back(QPoint(500,250));
    summonPos.push_back(QPoint(700,250));
    //设置每关每波次的敌人数据

    enemyLevelData[0][0] = {{1,2,5,14,8,25},{2,5,5,14,8,25},{3,7,8,14,8,25}};
    enemyLevelData[0][1] = {{2,3,7,14,8,25},{3,4,8,15,8,25},{5,7,9,14,8,25},{7,8,10,18,8,25},{5,5,5,14,8,50},{8,9,15,14,8,50}};
    enemyLevelData[0][2] = {{3,5,5,15,8,25},{4,5,7,14,8,25},{5,8,5,14,8,25},{6,2,5,14,8,25},{9,2,5,14,8,50}};
    enemyLevelData[1][0] = {{4,2,5,14,8,25},{1,2,5,14,8,25},{5,7,5,14,8,25},{4,2,5,14,8,25},{2,2,5,14,8,50}};
    enemyLevelData[1][1] = {{5,2,5,14,8,100}};
    enemyLevelData[1][2] = {{2,2,5,14,8,25},{3,4,5,14,8,25},{3,3,5,14,8,25},{5,5,5,14,8,25}};
    enemyLevelData[2][0] = {{5,2,5,14,8,25},{2,5,5,14,8,25},{5,2,5,14,8,25},{5,6,5,14,8,25},{5,2,7,14,8,25},{3,4,10,14,8,50}};
    enemyLevelData[2][1] = {{5,2,5,14,8,25},{4,5,5,14,8,25},{1,2,5,14,8,25},{4,6,5,14,8,25},{8,9,9,14,8,25},{9,9,9,14,15,50},{9,9,9,14,15,50},{9,9,9,14,15,50}};
    enemyLevelData[2][2] = {{1,2,5,14,8,25},{1,3,5,14,8,25}};

    //敌人指针绑定,敌人顺序如下
    enemyPtr[0] = &Enemy::blueSlimeAnimation;
    enemyPtr[1] = &Enemy::greenSlimeAnimation;
    enemyPtr[2] = &Enemy::flyingMonsterAnimation;
    enemyPtr[3] = &Enemy::wizardAnimation;
    enemyPtr[4] = &Enemy::assassinAnimation;
    enemyPtr[5] = &Enemy::soulAnimation;
    return;
}

bool GameMap::inBossRoom(){
    return curLevel == QPoint(1,1);
}

void GameMap::updateBgm(){
    if(!visit[curLevel.x()][curLevel.y()] && !playDoorSound[curLevel.x()][curLevel.y()]){
        playDoorSound[curLevel.x()][curLevel.y()] = true;
        AudioManager::instance()->playSound(AudioManager::DOOR_CLOSE);
        emit movePetToPlayer();
    }
    if(inBossRoom() && !m_startBossLevel && !m_finishBossLevel){
        m_startBossLevel = true;
    }
    return;
}
