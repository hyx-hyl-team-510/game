#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QStackedWidget>
#include <QLabel>
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QVector>
#include <QMap>
#include <memory>
#include <queue>
#include <vector>
#include "shop.h"
#include "pet.h"
#include "item.h"
#include "effect.h"
#include "wall.h"
#include "gamemap.h"

QT_BEGIN_NAMESPACE
class QPushButton;
class QGraphicsRectItem;
QT_END_NAMESPACE

class Player;
class Enemy;
class GameApplication;
class Projectile;
class InteractiveObject;

class GameWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GameWindow(GameApplication* app, QWidget *parent = nullptr);
    ~GameWindow();

    // 尺寸接口
    static int getWindowWidth() { return WINDOW_WIDTH; }
    static int getWindowHeight() { return WINDOW_HEIGHT; }
    static int getGameViewWidth() { return GAME_VIEW_WIDTH; }
    static int getGameViewHeight() { return GAME_VIEW_HEIGHT; }
    static int getSceneLeft() { return SCENE_LEFT; }
    static int getSceneRight() { return SCENE_RIGHT; }
    static int getSceneTop() { return SCENE_TOP; }
    static int getSceneBottom() { return SCENE_BOTTOM; }

    // 摄像机平滑参数
    static qreal getCameraSmoothFactor() { return CAMERA_SMOOTH_FACTOR; }
    static void setCameraSmoothFactor(qreal factor) { CAMERA_SMOOTH_FACTOR = factor; }

    // 游戏状态控制
    void startGame();
    void pauseGame();
    void resumeGame();
    void endGame();

    // 场景管理接口
    QGraphicsScene* getGameScene() const { return m_gameScene; }

    Player* getPlayer() const { return m_player; }
    // 摄像机接口
    void setCameraTarget(QGraphicsItem* target);
    void moveCameraTarget(QGraphicsPixmapItem* target);
    void updateCameraSmooth();

    // 输入处理接口
    void handlePlayerInput(int key, bool pressed);
    // 宠物接口
    void summonPet(Pet::PetType type, int level = 1);
    void dismissPet();
    Pet* getPet() const { return m_pet; }

signals:
    void gameStarted();
    void gamePaused();
    void gameResumed();
    void gameEnded();
    void levelLoaded(int levelId);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onStartButtonClicked();
    void onGameLoop();
    void onPlayerDamageDealt(Player* source, Enemy* target, int damage);
    void onEnemyDamageDealt(Enemy* source, Player* target, int damage);

private:
    void setupUI();
    void setupGameWorld();
    void connectSignals();

    // 窗口尺寸常量
    static constexpr int WINDOW_WIDTH = 960;
    static constexpr int WINDOW_HEIGHT = 640;
    static constexpr int GAME_VIEW_WIDTH = 960;
    static constexpr int GAME_VIEW_HEIGHT = 640;
    static constexpr int SCENE_LEFT = 0;
    static constexpr int SCENE_RIGHT = 3199;
    static constexpr int SCENE_TOP = 0;
    static constexpr int SCENE_BOTTOM = 3199;
    static constexpr int SCENE_WIDTH = 3200;
    static constexpr int SCENE_HEIGHT = 3200;
    void resolveWallCollision(Entity* entity);
    // 摄像机平滑参数静态变量
    static qreal CAMERA_SMOOTH_FACTOR;

    // UI组件
    QPushButton* m_startButton;
    QPushButton* m_nextButton;
    QGraphicsView* m_gameView;
    QGraphicsScene* m_gameScene;

    // 游戏对象
    Player* m_player;
    QGraphicsItem* m_cameraTarget;

    // 摄像机平滑状态
    QPointF m_currentCameraCenter;

    // 游戏循环
    QTimer* m_gameTimer;
    bool m_isGameRunning;
    bool m_isPaused;

    // 引用
    GameApplication* m_gameApp;
    static constexpr qreal AUTO_AIM_RANGE = 300.0;
    Shop *m_shop;//商店系统

    Pet* m_pet;
    QLabel* m_weaponLabel;  // 左下角武器图片
    QLabel* m_healthPotionLabel;
    QLabel* m_manaPotionLabel;
    QLabel* m_fireballLabel;
    QLabel* m_hpBar;
    QLabel* m_hpBg;
    QLabel* m_manaBar;
    QLabel* m_manaBg;
    QLabel* m_skillBar;
    QLabel* m_skillBg;
    QLabel* m_goldLabel;
    QLabel* m_transitionLabel;
    void openShop();//打开商店
    void updateWeaponDisplay();            // 更新武器显示
    void updateItemDisplay(Item::ItemType type);//更新道具显示

    void performPlayerAttack();//执行玩家攻击
    void performEnemyAttack();//执行敌人攻击
    void updateAutoAim();//在gameloop中更新自动瞄准
    QPixmap createIconWithNumber(QPixmap & icon, int number);
    QLabel* buildLabel(int x,int y);//新建label
    int itemDisplayWidth = 60;//显示图片的尺寸
    int itemDisplayHeight = 60;

    QStackedWidget* m_stackedWidget;
    QWidget* m_menuPage;    // 主菜单页
    QWidget* m_gamePage;    // 游戏页
    QWidget* m_settingsPage;  // 设置页
    void setupMenuPage();   // 主菜单界面
    void setupGamePage();   // 游戏界面
    void setupSettingsPage();  // 设置页面
    void switchToGame();    // 切换到游戏
    void switchToMenu();    // 切换回菜单
    void switchToSettings();   // 切换到设置

    void updateBar();//更新血条，魔法条
    void updateGold();//更新金币数量
    void worldReverse();//世界转变
    void transitionBase();//转场动画
    void transitionIn();
    std::vector<std::vector<bool>> m_visit;//转场动画数据
    std::queue<QPoint> m_queue;
    QPixmap* m_transitionPixmap;
    QPixmap* transitionSun;
    QPixmap* transitionMoon;
    QPixmap* transPix;
    QTimer m_transitionInTimer;
    bool inSun = true;
    bool inMoon = false;

    void timeStopSkill();//时停技能
    void timeStopSkillOver();
    void drawLight();
    void drawLightBoss();//boss房开场用的
    void startBossLevel();//开始boss关的一些提示，如警报
    void finishBossLevel();//结束boss关，关闭boss关音乐之类的
    void updateBossLevel();
    void createAIWeapon();//创建AI武器
    void createAITalk();//创建AI对话
    void showFloatingText(QPointF worldPos,QString text,QColor color = Qt::white);
    bool isTimeStop = false;
    std::queue<QPointF> m_timeStopQueue;
    QTimer m_timeStopTimer;
    QTimer m_timeStopOverTimer;
    QGraphicsPixmapItem* m_darknessItem;
    QPixmap* m_darknessMask;

    QTimer m_bossRoomTimer;//对应计时器
    QGraphicsPixmapItem* bossRoomCenter = nullptr;//为了实现摄像机移动

    QTimer m_bossTalkTimer;//boss对话计时器
};

#endif // GAMEWINDOW_H
