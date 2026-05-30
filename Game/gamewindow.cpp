#include "GameWindow.h"
#include "GameApplication.h"
#include "audiomanager.h"
#include "aimanager.h"
#include "interactiveobject.h"
#include "Player.h"
#include "Enemy.h"
#include "weapon.h"
#include "CombatSystem.h"
#include "Projectile.h"
#include "entitymanager.h"
#include "potion.h"
#include "throwable.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QKeyEvent>
#include <QDebug>
#include <QSlider>
#include <QLineEdit>
#include <QCheckBox>
// 初始化摄像机平滑参数
qreal GameWindow::CAMERA_SMOOTH_FACTOR = 0.1;

GameWindow::GameWindow(GameApplication* app, QWidget *parent)
    : QMainWindow(parent)
    , m_startButton(nullptr)
    , m_nextButton(nullptr)
    , m_gameView(nullptr)
    , m_gameScene(nullptr)
    , m_player(nullptr)
    , m_cameraTarget(nullptr)
    , m_gameTimer(nullptr)
    , m_isGameRunning(false)
    , m_isPaused(false)
    , m_gameApp(app)
    , m_currentCameraCenter(0, 0)
    , m_pet(nullptr)
{
    setupUI();
    connectSignals();
    setWindowTitle("超时空勇士:光痕回响");
    setWindowIcon(QIcon("resource/icon/icon.png"));
    setFixedSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    setFocusPolicy(Qt::StrongFocus);
    m_shop = new Shop(this);
    // 添加商品
    Potion* health_potion = new Potion(Item::POTION,"生命药水",Item::healthPotionIcon(),Potion::HEALTH_POTION);
    Potion* mana_potion = new Potion(Item::POTION,"魔法药水",Item::manaPotionIcon(),Potion::MANA_POTION);
    Throwable* fireball = new Throwable(Item::THROWABLE,"火球",Item::fireballIcon(),Throwable::FIREBALL);
    m_shop->addItem(health_potion,50);
    m_shop->addItem(mana_potion,50);
    m_shop->addItem(fireball,50);
    connect(m_shop, &Shop::gamePauseRequested, this, &GameWindow::pauseGame);
    connect(m_shop, &Shop::gameResumeRequested, this, &GameWindow::resumeGame);
    connect(m_shop,&Shop::itemPurchased,this,&GameWindow::updateWeaponDisplay);
    connect(m_shop,&Shop::updateItemDisplay,this,&GameWindow::updateItemDisplay);
    connect(GameMap::instance(),&GameMap::createAIWeapon,this,&GameWindow::createAIWeapon);
    connect(GameMap::instance(),&GameMap::createAITalk,this,&GameWindow::createAITalk);
    connect(GameMap::instance(),&GameMap::movePetToPlayer,this,[this](){
        if(m_pet && m_pet->Entity::isActive()){
            m_pet->Entity::setPos(m_player->pos() + QPointF(1,1));
        }
    });
    Weapon::InitIcons();
    Enemy::combatSystem = m_gameApp->getCombatSystem();
}

GameWindow::~GameWindow()
{
    if (m_gameTimer) {
        m_gameTimer->stop();
    }
}

void GameWindow::setupUI()
{
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_stackedWidget = new QStackedWidget();
    mainLayout->addWidget(m_stackedWidget);

    setupMenuPage();   // 第 0 页
    setupGamePage();   // 第 1 页
    setupSettingsPage();  // 第 2 页

    m_stackedWidget->setCurrentIndex(0);  // 默认显示主菜单

    setCentralWidget(centralWidget);
}

void GameWindow::setupMenuPage()
{
    m_menuPage = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_menuPage);
    layout->setAlignment(Qt::AlignCenter);

    // 标题
    QLabel* title = new QLabel("超时空勇士:光痕回响");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 36px; color: white; font-weight: bold;");
    layout->addWidget(title);
    layout->addSpacing(50);

    // 开始按钮
    QPushButton* startBtn = new QPushButton("开始游戏");
    startBtn->setFixedSize(100, 40);
    startBtn->setStyleSheet(
        "font-size: 20px;"
        "color: white;"
        "border: none;"
        "background-color: transparent;"
        );
    connect(startBtn, &QPushButton::clicked, this, &GameWindow::switchToGame);
    layout->addWidget(startBtn, 0, Qt::AlignCenter);
    layout->addSpacing(20);

    // 设置按钮
    QPushButton* settingsBtn = new QPushButton("设置");
    settingsBtn->setFixedSize(200, 50);
    settingsBtn->setStyleSheet("font-size: 20px; color: white; border: none; background-color: transparent;");
    connect(settingsBtn, &QPushButton::clicked, this, &GameWindow::switchToSettings);
    layout->addWidget(settingsBtn, 0, Qt::AlignCenter);
    layout->addSpacing(20);

    // 退出按钮
    QPushButton* quitBtn = new QPushButton("退出游戏");
    quitBtn->setFixedSize(100, 40);
    quitBtn->setStyleSheet(
        "font-size: 20px;"
        "color: white;"
        "border: none;"
        "background-color: transparent;"
        );
    connect(quitBtn, &QPushButton::clicked, this, &QMainWindow::close);
    layout->addWidget(quitBtn, 0, Qt::AlignCenter);
    layout->addSpacing(50);

    // 背景
    QLabel* paper = new QLabel(m_menuPage);
    paper->setPixmap(QPixmap("resource/paper/main_interface.png"));
    paper->adjustSize();
    paper->move(0, 0);
    paper->lower();
    m_stackedWidget->addWidget(m_menuPage);  // 索引 0
}

void GameWindow::setupGamePage()
{
    m_gamePage = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(m_gamePage);
    layout->setContentsMargins(0, 0, 0, 0);

    m_gameView = new QGraphicsView();
    m_gameScene = new QGraphicsScene();
    m_gameScene->setSceneRect(SCENE_LEFT, SCENE_TOP, SCENE_WIDTH, SCENE_HEIGHT);
    m_gameView->setScene(m_gameScene);
    m_gameView->setFixedSize(GAME_VIEW_WIDTH, GAME_VIEW_HEIGHT);
    m_gameView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_gameView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_gameView->setRenderHint(QPainter::Antialiasing);
    m_gameView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_gameView->setFrameShape(QFrame::NoFrame);
    m_gameView->setFocusPolicy(Qt::NoFocus);
    m_gameView->viewport()->installEventFilter(this);
    layout->addWidget(m_gameView);

    m_stackedWidget->addWidget(m_gamePage);  // 索引 1
}

void GameWindow::setupSettingsPage()
{
    m_settingsPage = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_settingsPage);
    layout->setAlignment(Qt::AlignCenter);

    // 标题
    QLabel* title = new QLabel("设置");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 36px; color: white; font-weight: bold;");
    layout->addWidget(title);
    layout->addSpacing(40);

    // 音量滑块
    QLabel* bgmLabel = new QLabel("背景音乐音量");
    bgmLabel->setStyleSheet("color: white; font-size: 18px;");
    layout->addWidget(bgmLabel);

    QSlider* bgmSlider = new QSlider(Qt::Horizontal);
    bgmSlider->setRange(0, 100);
    bgmSlider->setValue(100);
    layout->addWidget(bgmSlider);

    QLabel* sfxLabel = new QLabel("音效音量");
    sfxLabel->setStyleSheet("color: white; font-size: 18px;");
    layout->addWidget(sfxLabel);

    QSlider* sfxSlider = new QSlider(Qt::Horizontal);
    sfxSlider->setRange(0, 100);
    sfxSlider->setValue(100);
    layout->addWidget(sfxSlider);

    layout->addSpacing(40);

    connect(bgmSlider, &QSlider::valueChanged, this, [](int val) {
        AudioManager::instance()->setBGMVolume(val / 100.0f);
    });

    connect(sfxSlider, &QSlider::valueChanged, this, [](int val) {
        AudioManager::instance()->setSFXVolume(val / 100.0f);
    });

    // Key输入框
    QLabel* keyLabel = new QLabel("API-Key(目前只支持deepseek)");
    keyLabel->setStyleSheet("color: white; font-size: 18px;");
    layout->addWidget(keyLabel);
    QLineEdit* keyEdit = new QLineEdit;
    keyEdit->setPlaceholderText("请输入Key");
    keyEdit->setFixedWidth(750);
    keyEdit->setStyleSheet("font-size: 16px; padding: 5px; color: white;");
    connect(keyEdit, &QLineEdit::editingFinished, this, [keyEdit]() {
        AIManager::instance()->setKey(keyEdit->text());
        qDebug() << "Key = " << keyEdit->text();
    });

    //勾选框
    QCheckBox* enableCheckBox = new QCheckBox("启用AI功能");
    enableCheckBox->setStyleSheet(
        "QCheckBox { color: white; font-size: 16px; }"
        "QCheckBox::indicator {"
        "  border: 2px solid white;"
        "  border-radius: 3px;"
        "  width: 18px;"
        "  height: 18px;"
        "}"
        "QCheckBox::indicator:checked {"
        "  background-color: #4CAF50;"          // 绿色底
        "  border-color: white;"
        "}"
        );
    connect(enableCheckBox, &QCheckBox::stateChanged, this, [](int state) {
        AIManager::instance()->setEnabled(state == Qt::Checked);
        qDebug() << (AIManager::instance()->isEnabled() ? "开启" : "关闭");
    });
    QHBoxLayout* keyRow = new QHBoxLayout;
    keyRow->addWidget(keyEdit);
    keyRow->addWidget(enableCheckBox);
    layout->addLayout(keyRow);

    // 返回按钮
    QPushButton* backBtn = new QPushButton("返回主菜单");
    backBtn->setFixedSize(200, 50);
    backBtn->setStyleSheet(
        "font-size: 20px;"
        "color: white;"
        "border: none;"
        "background-color: transparent;"
        );
    connect(backBtn, &QPushButton::clicked, this, [this]() {
        m_stackedWidget->setCurrentIndex(0);  // 回到菜单
    });
    layout->addWidget(backBtn, 0, Qt::AlignCenter);
    m_settingsPage->setStyleSheet("background-color: #1a1a2e;");
    m_stackedWidget->addWidget(m_settingsPage);  // 索引 2
}

void GameWindow::setupGameWorld()
{
    GameMap::instance()->InitMap("resource/Map/gameMap.png",QPoint(SCENE_LEFT,SCENE_TOP));
    m_gameScene->addItem(GameMap::instance()->getMap());
    Wall* wall[16];
    wall[0] = EntityManager::instance()->createEntity<Wall>(50,0,3100,50);
    wall[1] = EntityManager::instance()->createEntity<Wall>(50,3150,3100,50);
    wall[2] = EntityManager::instance()->createEntity<Wall>(0,50,50,3100);
    wall[3] = EntityManager::instance()->createEntity<Wall>(3150,50,50,3100);
    wall[4] = EntityManager::instance()->createEntity<Wall>(1050,50,50,2500);
    wall[5] = EntityManager::instance()->createEntity<Wall>(2100,650,50,2500);
    wall[6] = EntityManager::instance()->createEntity<Wall>(650,1050,850,50);
    wall[7] = EntityManager::instance()->createEntity<Wall>(1700,1050,850,50);
    wall[8] = EntityManager::instance()->createEntity<Wall>(650,2100,850,50);
    wall[9] = EntityManager::instance()->createEntity<Wall>(1700,2100,850,50);
    wall[10] = EntityManager::instance()->createEntity<Wall>(2100,50,50,400);
    wall[11] = EntityManager::instance()->createEntity<Wall>(50,1050,400,50);
    wall[12] = EntityManager::instance()->createEntity<Wall>(2750,1050,400,50);
    wall[13] = EntityManager::instance()->createEntity<Wall>(50,2100,400,50);
    wall[14] = EntityManager::instance()->createEntity<Wall>(2750,2100,400,50);
    wall[15] = EntityManager::instance()->createEntity<Wall>(1050,2750,50,400);
    for(int i=0;i<16;i++){
        EntityManager::instance()->addToScene(wall[i]);
    }
}

void GameWindow::switchToGame()
{
    m_stackedWidget->setCurrentIndex(1);
    startGame();  // 原来的游戏启动逻辑
}

void GameWindow::switchToMenu()
{
    m_stackedWidget->setCurrentIndex(0);
    endGame();  // 停止游戏循环
}

void GameWindow::switchToSettings()
{
    m_stackedWidget->setCurrentIndex(2);  // 设置页索引
}

void GameWindow::startGame()
{
    if (m_isGameRunning) return;
    //预热网络连接
    // ========== 初始化 EntityManager ==========
    EntityManager::instance()->setScene(m_gameScene);
    EntityManager::instance()->clearAll();
    // 1. 创建玩家
    m_player = EntityManager::instance()->createEntity<Player>(0,0);
    m_player->setTeam(EntityTeam::PLAYER);
    m_player->setPos(200, 200);
    // 2. 设置场景（必须先于添加其他实体）
    EntityManager::instance()->addToScene(m_player);
    EntityManager::instance()->setPlayer(m_player);
    connect(m_player,&Player::performAttack,this,&GameWindow::performPlayerAttack);
    connect(m_player,&Player::updateGold,this,&GameWindow::updateGold);
    connect(&m_timeStopTimer,&QTimer::timeout,this,[this](){
        m_player->update(16.0f);
        resolveWallCollision(m_player);
        drawLight();
        updateCameraSmooth();
    });
    connect(&m_timeStopOverTimer,&QTimer::timeout,this,&GameWindow::timeStopSkillOver);
    connect(&m_bossRoomTimer,&QTimer::timeout,this,[this](){
        updateCameraSmooth();
        drawLightBoss();
    });
    connect(&m_bossTalkTimer,&QTimer::timeout,this,&GameWindow::createAITalk);
    setupGameWorld();

    // 3. 关卡加载完成后，再召唤宠物
    summonPet(Pet::PET_PICAQU, 3);
    // 4. 启动游戏循环
    m_isGameRunning = true;
    m_isPaused = false;

    if (m_player) {
        m_currentCameraCenter = m_player->pos();
    }

    if (!m_gameTimer) {
        m_gameTimer = new QTimer(this);
        connect(m_gameTimer, &QTimer::timeout, this, &GameWindow::onGameLoop);
    }
    m_gameTimer->start(16);
    //创建武器，道具label
    m_weaponLabel = buildLabel(10,GAME_VIEW_HEIGHT - 70);
    m_healthPotionLabel = buildLabel(GAME_VIEW_WIDTH - 210,GAME_VIEW_HEIGHT - 70);
    m_manaPotionLabel = buildLabel(GAME_VIEW_WIDTH - 140,GAME_VIEW_HEIGHT - 70);
    m_fireballLabel = buildLabel(GAME_VIEW_WIDTH - 70,GAME_VIEW_HEIGHT - 70);
    m_player->getItems().health_potion_count = 3;
    m_player->getItems().mana_potion_count = 3;
    m_player->getItems().fireball_count = 1000;
    m_healthPotionLabel->setPixmap(createIconWithNumber(Item::healthPotionIcon(),m_player->getItems().health_potion_count));
    m_manaPotionLabel->setPixmap(createIconWithNumber(Item::manaPotionIcon(),m_player->getItems().mana_potion_count));
    m_fireballLabel->setPixmap(createIconWithNumber(Item::fireballIcon(),m_player->getItems().fireball_count));

    Weapon* sword = new Weapon(Item::WEAPON,"勇者之剑",Weapon::baseSwordIcon(),Weapon::SWORD,10,1);
    Weapon* bow = new Weapon(Item::WEAPON,"勇者之弓",Weapon::baseBowIcon(),Weapon::BOW,10,1);
    m_player->addWeapon(sword);
    m_player->addWeapon(bow);
    updateWeaponDisplay();
    //创建血条,魔法条

    // 血条（红色，宽度动态变化）
    m_hpBg = new QLabel(m_gameView);
    m_hpBg->setFixedSize(200, 25);
    m_hpBg->move(100, GAME_VIEW_HEIGHT - 70);
    m_hpBg->setPixmap(QPixmap("resource/Bar/hpBg.png"));
    m_hpBg->setFrameShape(QFrame::NoFrame);
    m_hpBg->show();

    m_hpBar = new QLabel(m_gameView);
    m_hpBar->setFixedSize(200, 25);
    m_hpBar->move(100, GAME_VIEW_HEIGHT - 70);
    m_hpBar->setPixmap(QPixmap("resource/Bar/hpBar.png"));
    m_hpBar->setFrameShape(QFrame::NoFrame);
    m_hpBar->show();
    //魔法条
    m_manaBg = new QLabel(m_gameView);
    m_manaBg->setFixedSize(200,25);
    m_manaBg->move(100,GAME_VIEW_HEIGHT - 35);
    m_manaBg->setPixmap(QPixmap("resource/Bar/manaBg.png"));
    m_manaBg->setFrameShape(QFrame::NoFrame);
    m_manaBg->show();

    m_manaBar = new QLabel(m_gameView);
    m_manaBar->setFixedSize(200,25);
    m_manaBar->move(100,GAME_VIEW_HEIGHT - 35);
    m_manaBar->setPixmap(QPixmap("resource/Bar/manaBar.png"));
    m_manaBar->setFrameShape(QFrame::NoFrame);
    m_manaBar->show();
    //技能冷却条
    m_skillBg = new QLabel(m_gameView);
    m_skillBg->setFixedSize(20,60);
    m_skillBg->move(320,GAME_VIEW_HEIGHT - 70);
    m_skillBg->setPixmap(QPixmap("resource/Bar/skillBg.png"));
    m_skillBg->setFrameShape(QFrame::NoFrame);
    m_skillBg->show();

    m_skillBar = new QLabel(m_gameView);
    m_skillBar->setFixedSize(20,60);
    m_skillBar->move(320,GAME_VIEW_HEIGHT - 70);
    m_skillBar->setPixmap(QPixmap("resource/Bar/skillBar.png"));
    m_skillBar->setFrameShape(QFrame::NoFrame);
    m_skillBar->show();

    //金币条
    m_goldLabel = new QLabel(m_gameView);
    m_goldLabel->setFixedSize(100,20);
    m_goldLabel->move(GAME_VIEW_WIDTH - 210,GAME_VIEW_HEIGHT - 100);
    m_goldLabel->setFrameShape(QFrame::NoFrame);
    m_goldLabel->show();
    updateGold();
    //转场标签
    m_transitionLabel = new QLabel(m_gamePage);
    m_transitionLabel->setStyleSheet("background: transparent; border: none;");
    m_transitionLabel->setFixedSize(GAME_VIEW_WIDTH,GAME_VIEW_HEIGHT);
    m_transitionLabel->raise();
    m_transitionPixmap = new QPixmap(GAME_VIEW_WIDTH,GAME_VIEW_HEIGHT);
    transitionSun = new QPixmap;
    transitionSun->load("resource/Transition/sun.png");
    transitionMoon = new QPixmap;
    transitionMoon->load("resource/Transition/moon.png");
    connect(&m_transitionInTimer,&QTimer::timeout,this,&GameWindow::transitionIn);
    //遮罩图片
    m_darknessItem = new QGraphicsPixmapItem;
    m_darknessItem->setPos(QPoint(0,0));
    m_darknessItem->setZValue(1000);
    m_darknessMask = new QPixmap(GAME_VIEW_WIDTH,GAME_VIEW_HEIGHT);
    m_darknessMask->load("resource/paper/light_mask.png");
    m_darknessItem->setPixmap(*m_darknessMask);
    m_darknessItem->hide();
    m_gameScene->addItem(m_darknessItem);

    //boss房摄像机目标
    bossRoomCenter = new QGraphicsPixmapItem;
    bossRoomCenter->setPos(SCENE_WIDTH / 2,SCENE_HEIGHT / 2);

    //初始化商店图片
    m_shop->setBackgroundImage("resource/Shop/shop.png");
    m_shop->setCellBackgroundImage(":/images/shop_cell.png");
    m_shop->setButtonImage(":/images/btn_buy.png");
    m_shop->setButtonHoverImage(":/images/btn_buy_hover.png");
    m_shop->setTitleImage(":/images/shop_title.png");

    m_player->loadAnimation("death","resource/Character/death.png",80,80,1);
    m_player->loadAnimation("attack_sword","resource/Character/Sword_strip6.png",80,80,6);
    m_player->loadAnimation("attack_bow","resource/Character/Bow_strip6.png",80,80,6);
    m_player->loadAnimation("idle_sword","resource/Character/Idle_strip4.png",80,80,4);
    m_player->loadAnimation("idle_bow","resource/Character/Idle_bow.png",80,80,4);
    m_player->loadAnimation("walk_left_bow","resource/Character/Run_bow4.png",80,80,4);
    m_player->loadAnimation("walk_right_bow","resource/Character/Run_bow4.png",80,80,4);
    m_player->loadAnimation("walk_up_bow","resource/Character/Run_bow4.png",80,80,4);
    m_player->loadAnimation("walk_down_bow","resource/Character/Run_bow4.png",80,80,4);

    m_player->loadAnimation("walk_left_sword","resource/Character/Run_strip4.png",80,80,4);
    m_player->loadAnimation("walk_right_sword","resource/Character/Run_strip4.png",80,80,4);
    m_player->loadAnimation("walk_up_sword","resource/Character/Run_strip4.png",80,80,4);
    m_player->loadAnimation("walk_down_sword","resource/Character/Run_strip4.png",80,80,4);
    setCameraTarget(m_player);
    emit gameStarted();
    qDebug() << "Game started with pet";
}

void GameWindow::pauseGame()
{
    if (!m_isGameRunning || m_isPaused) return;
    m_isPaused = true;
    if (m_gameTimer) m_gameTimer->stop();
    emit gamePaused();
}

void GameWindow::resumeGame()
{
    if (!m_isGameRunning || !m_isPaused) return;
    m_isPaused = false;
    if (m_gameTimer) m_gameTimer->start(16);
    emit gameResumed();
}

void GameWindow::endGame()
{
    if (!m_isGameRunning) return;
    m_isGameRunning = false;
    if (m_gameTimer) m_gameTimer->stop();
    emit gameEnded();
}

void GameWindow::setCameraTarget(QGraphicsItem* target)
{
    m_cameraTarget = target;
    if (target) {
        m_currentCameraCenter = target->pos();
    }
}

void GameWindow::moveCameraTarget(QGraphicsPixmapItem* target){
    m_cameraTarget = target;
    return;
}

void GameWindow::updateCameraSmooth()
{
    if (!m_cameraTarget || !m_gameView) return;

    QPointF targetPos = m_cameraTarget->pos();
    QRectF viewRect = m_gameScene->sceneRect();

    qreal targetX = qBound(viewRect.left() + GAME_VIEW_WIDTH / 2,
                           targetPos.x(),
                           viewRect.right() - GAME_VIEW_WIDTH / 2);
    qreal targetY = qBound(viewRect.top() + GAME_VIEW_HEIGHT / 2,
                           targetPos.y(),
                           viewRect.bottom() - GAME_VIEW_HEIGHT / 2);
    QPointF targetCenter(targetX, targetY);

    m_currentCameraCenter = m_currentCameraCenter + (targetCenter - m_currentCameraCenter) * CAMERA_SMOOTH_FACTOR;
    m_gameView->centerOn(m_currentCameraCenter);
}

void GameWindow::handlePlayerInput(int key, bool pressed)
{
    if (!m_player || !m_isGameRunning || m_isPaused) return;

    // 只在按键按下时处理（释放按键不处理）
    if (!pressed) return;

    if(key == Qt::Key_P){//功能测试按键
        QVector<Enemy*> enemies = EntityManager::instance()->getEntitiesByType<Enemy>();
        for(Enemy* e : enemies){
            if(e && e->isActive()){
                e->kill();
            }
        }
        EntityManager::instance()->destroyAllEnemies();
    }

    // 近战攻击 J
    if (key == Qt::Key_J) {
        qDebug() << "J";
        m_player->attack();
    }
    //时停技能
    if (key == Qt::Key_L){
        timeStopSkill();
    }
    // 武器切换 K（下一把）
    if (key == Qt::Key_K) {
        if (m_player->switchToNextWeapon()) {
            updateWeaponDisplay();  // 更新显示
            qDebug() << "Switched to next weapon";
        }
    }
    //卸下武器
    if (key == Qt::Key_G) {
        m_player->removeWeapon();
        updateWeaponDisplay();  // 更新显示
        qDebug() << "Weapon unequipped";
    }

    //使用道具
    if(key == Qt::Key_U){
        if(m_player->getItems().health_potion_count > 0){
            Entity* heal = EntityManager::instance()->createEntity<Effect>(m_player->pos(),Effect::HEAL);
            EntityManager::instance()->addToScene(heal);
            qDebug()<<"使用了生命药水";
            m_player->heal(Item::HEALTH_POTION_EFFECT);
            m_player->getItems().health_potion_count--;
            updateItemDisplay(Item::POTION);
        }else{
            qDebug()<<"生命药水数量不足";
        }
    }
    if(key == Qt::Key_I){
        if(m_player->getItems().mana_potion_count > 0){
            Entity* mana = EntityManager::instance()->createEntity<Effect>(m_player->pos(),Effect::MANA);
            EntityManager::instance()->addToScene(mana);
            qDebug()<<"使用了魔法药水";
            m_player->setMana(m_player->getMana() + Item::MANA_POTION_EFFECT);
            m_player->getItems().mana_potion_count--;
            updateItemDisplay(Item::POTION);
        }else{
            qDebug()<<"魔法药水数量不足";
        }
    }
    if(key == Qt::Key_O){
        if(m_player->getItems().fireball_count > 0){
            qDebug()<<"使用了火球";
            m_player->getItems().fireball_count--;
            updateItemDisplay(Item::THROWABLE);
            CombatSystem *combat = m_gameApp->getCombatSystem();
            if(combat){
                combat->castFireball(m_player);
            }
        }else{
            qDebug()<<"火球数量不足";
        }
    }
    if(key == Qt::Key_T){//世界转换
        worldReverse();
    }
}
void GameWindow::onStartButtonClicked()
{
    qDebug() << "Start button clicked - starting game";
    startGame();
}

void GameWindow::onGameLoop()
{
    if (!m_isGameRunning || m_isPaused) return;
    drawLight();
    // 1. 更新所有实体（包括玩家、宠物、敌人、子弹）
    EntityManager::instance()->updateAll(16.0f);
    updateAutoAim();//更新自动瞄准
    // 2. 处理玩家边界（如果玩家还没完全迁移到 EntityManager）
    if (m_player) {
        resolveWallCollision(m_player);
    }

    GameMap::instance()->updateLevel(m_player->pos(),m_player->getWidth(),m_player->getHeight());
    updateBossLevel();
    // 敌人墙壁碰撞
    QVector<Entity*> enemies = EntityManager::instance()->getEntitiesByType(EntityType::ENEMY);
    for (Entity* e : enemies) {
        resolveWallCollision(e);
    }

    // 宠物墙壁碰撞
    if (m_pet) {
        resolveWallCollision(m_pet);
    }

    // 3. 处理延迟操作
    EntityManager::instance()->processPendingOperations();

    // 4. 检查玩家死亡
    updateBar();//更新
    if (m_player && m_player->isDead()) {
        endGame();
        return;
    }
    // 更新武器显示位置（保持在屏幕左下角）
    // 5. 摄像机跟随
    updateCameraSmooth();
}

void GameWindow::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    // 按 B 键打开/关闭商店
    if (key == Qt::Key_B) {
        InteractiveObject* shopItem = static_cast<InteractiveObject*>(EntityManager::instance()->getShopItem());
        if(!shopItem || !shopItem->isInteractive(m_player->pos())){return;}//没创建商店或不可用
        qDebug() << "B pressed, shop isVisible =" << m_shop->isVisible();
        if (!m_shop->isVisible()) {
            qDebug() << "Calling openShop()";
            openShop();
        } else {
            qDebug() << "Calling shop->hide()";
            m_shop->hide();
        }
        return;
    }
    // 移动按键：传递给 Player 的按键状态系统
    if (key == Qt::Key_W || key == Qt::Key_S
        ||
        key == Qt::Key_A || key == Qt::Key_D) {
        if (m_player) {
            m_player->setKeyState(key, true);
        }
    }
    // 攻击和技能按键：直接处理
    else {
        handlePlayerInput(key, true);
    }
    if(isTimeStop && key == Qt::Key_J){//时停时发动攻击
        m_timeStopQueue.push(m_player->pos());
    }
    //QMainWindow::keyPressEvent(event);
}

void GameWindow::keyReleaseEvent(QKeyEvent *event)
{
    int key = event->key();
    // 移动按键：传递给 Player 的按键状态系统
    if (key == Qt::Key_W || key == Qt::Key_S
        ||
        key == Qt::Key_A || key == Qt::Key_D) {
        if (m_player) {
            m_player->setKeyState(key, false);
        }
    }

   // QMainWindow::keyReleaseEvent(event);
}

void GameWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (m_cameraTarget) {
        m_currentCameraCenter = m_cameraTarget->pos();
        updateCameraSmooth();
    }
}

bool GameWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_gameView->viewport() && event->type() == QEvent::Wheel)
        return true;
    return QMainWindow::eventFilter(obj, event);
}

void GameWindow::connectSignals()
{
    if (!m_gameApp) return;

    CombatSystem* combatSys = m_gameApp->getCombatSystem();
    if (combatSys) {
        connect(combatSys, &CombatSystem::playerDamageDealt,
                this, &GameWindow::onPlayerDamageDealt);
        connect(combatSys, &CombatSystem::enemyDamageDealt,
                this, &GameWindow::onEnemyDamageDealt);
    }
}

void GameWindow::onPlayerDamageDealt(Player* source, Enemy* target, int damage)
{
    qDebug() << "=== onPlayerDamageDealt START ===";
    Q_UNUSED(source);
    qDebug() << "Player dealt" << damage << "damage to enemy, enemy health:" << target->getHealth();
    qDebug() << "  target health:" << target->getHealth();
    qDebug() << "  target isDead:" << target->isDead();
    qDebug() << "=== onPlayerDamageDealt END ===";
}
void GameWindow::onEnemyDamageDealt(Enemy* source, Player* target, int damage)
{
    Q_UNUSED(source);
    qDebug() << "Enemy dealt" << damage << "damage to player, player health:" << target->getHealth();

    if (target->isDead()) {
        endGame();
    }
}

void GameWindow::openShop()
{
    if (!m_player) return;

    //m_shop->clearItems();
    int shopWidth = Shop::getGridCols() * Shop::getCellWidth();
    int shopHeight = Shop::getGridRows() * Shop::getCellHeight();

    // 屏幕坐标（相对于视图左上角）
    int screenX = (GAME_VIEW_WIDTH - shopWidth) / 2;
    int screenY = (GAME_VIEW_HEIGHT - shopHeight) / 2;

    // 传入 view 用于坐标转换
    m_shop->show(m_gameScene, m_gameView, screenX, screenY, m_player);
}

void GameWindow::summonPet(Pet::PetType type, int level)
{
    // 如果已有宠物，先移除
    if (m_pet) {
        return;
        //dismissPet();
    }

    // 检查玩家是否存在
    if (!m_player) {
        qDebug() << "summonPet: Player is null!";
        return;
    }

    // 通过 EntityManager 创建宠物
    m_pet = EntityManager::instance()->createEntity<Pet>(type, level);
    // 设置位置（在玩家旁边）
    m_pet->QGraphicsPixmapItem::setPos(m_player->pos().x() + 50, m_player->pos().y() + 50);

    // 设置队伍
    m_pet->setTeam(EntityTeam::PLAYER);

    // 添加到场景
    EntityManager::instance()->addToScene(m_pet);

    // 连接宠物信号
    connect(m_pet, &Pet::petAttackEnemy, this, [this](Pet* pet, Enemy* target, int damage) {
        qDebug() << "Pet attacked enemy for" << damage << "damage";
    });

    connect(m_pet, &Pet::petDied, this, [this](Pet* pet) {
        qDebug() << "Pet died!";
        // 5秒后复活
        QTimer::singleShot(5000, this, [this, pet]() {
            if (pet && pet == m_pet) {
                pet->revive();
                qDebug() << "Pet revived!";
            }
        });
    });
    qDebug() << "Summoned pet:" << m_pet->getTypeName() << "Level:" << level;
}

void GameWindow::dismissPet()
{
    if (m_pet) {
        EntityManager::instance()->destroyEntity(m_pet);
        m_pet = nullptr;
        qDebug() << "Pet dismissed";
    }
}
void GameWindow::updateWeaponDisplay()
{
    if (!m_weaponLabel || !m_player) return;

    Weapon* currentWeapon = m_player->getCurrentWeapon();

    if (currentWeapon && !currentWeapon->getIcon().isNull()) {
        QPixmap icon = currentWeapon->getIcon().scaled(itemDisplayWidth, itemDisplayHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_weaponLabel->setPixmap(icon);
    } else {
        m_weaponLabel->setText("空手");
        m_weaponLabel->setStyleSheet(
            "background-color: rgba(0, 0, 0, 120);"
            "border: 2px solid rgba(255, 255, 255, 80);"
            "color: white; font-size: 12px;"
            );
    }
}
void GameWindow::performPlayerAttack(){
    QPointF attackDirection = m_player->getAttackDirection();
    CombatSystem* combatSys = m_gameApp->getCombatSystem();
    if (combatSys) {
        if(m_player->getCurrentWeapon()->getType() == Weapon::SWORD){//用剑
            combatSys->meleeAttack(m_player, attackDirection,
                                   EntityManager::instance()->getEntitiesByType<Enemy>());
        }else if(m_player->getCurrentWeapon()->getType() == Weapon::BOW){//用弓
            combatSys->rangeAttack(m_player, attackDirection);
        }
    }
    return;
}
void GameWindow::performEnemyAttack(){

}

void GameWindow::updateAutoAim(){
    // 自动瞄准：获取最近的敌人
    if(!m_player->getTarget() || m_player->getTarget()->isDead()){
        Entity* target = EntityManager::instance()->getNearestEnemy(
            m_player->pos(), AUTO_AIM_RANGE
            );
        Enemy* targetEnemy = static_cast<Enemy*>(target);
        m_player->setTarget(targetEnemy);
    }
    return;
}

QLabel* GameWindow::buildLabel(int x,int y){
    // 创建 Label，放在 gameView 上层
    QLabel* label = new QLabel(m_gameView);
    label->setFixedSize(60, 60);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet(
        "background-color: rgba(0, 0, 0, 120);"
        "border: 2px solid rgba(255, 255, 255, 80);"
        );
    // 定位
    label->move(x,y);
    label->show();
    return label;
}

void GameWindow::updateItemDisplay(Item::ItemType type){
    ItemData items = m_player->getItems();
    switch(type){
        case Item::POTION:
            m_healthPotionLabel->setPixmap(createIconWithNumber(Item::healthPotionIcon(),items.health_potion_count));
            m_manaPotionLabel->setPixmap(createIconWithNumber(Item::manaPotionIcon(),items.mana_potion_count));
            break;
        case Item::THROWABLE:
            m_fireballLabel->setPixmap(createIconWithNumber(Item::fireballIcon(),items.fireball_count));
            break;
    }
    return;
}

QPixmap GameWindow::createIconWithNumber(QPixmap & icon, int number){
    // 扩大一点尺寸给数字留空间
    QPixmap result = icon.scaled(itemDisplayWidth,itemDisplayHeight,Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPainter painter(&result);
    // 绘制数字
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(1, 1, 25, 18, Qt::AlignCenter, QString::number(number));
    return result;
}

void GameWindow::resolveWallCollision(Entity* entity)
{
    if (!entity || !entity->isAlive()) return;

    QVector<Entity*> walls = EntityManager::instance()->getEntitiesByType(EntityType::WALL);
    QRectF entityRect = entity->getCollisionRect();
    QPointF newPos = entity->pos();

    for (Entity* w : walls) {
        if (!w->isVisible()) continue;//墙不可见
        QRectF wallRect = w->getCollisionRect();

        if (!entityRect.intersects(wallRect)) continue;

        // 计算四个方向的重叠量
        qreal overlapLeft   = entityRect.right()  - wallRect.left();
        qreal overlapRight  = wallRect.right()    - entityRect.left();
        qreal overlapTop    = entityRect.bottom() - wallRect.top();
        qreal overlapBottom = wallRect.bottom()   - entityRect.top();

        // 找最小重叠方向
        qreal minOverlap = qMin(qMin(overlapLeft, overlapRight),
                                qMin(overlapTop, overlapBottom));

        if (minOverlap == overlapLeft) {
            newPos.setX(newPos.x() - overlapLeft);
        } else if (minOverlap == overlapRight) {
            newPos.setX(newPos.x() + overlapRight);
        } else if (minOverlap == overlapTop) {
            newPos.setY(newPos.y() - overlapTop);
        } else {
            newPos.setY(newPos.y() + overlapBottom);
        }

        entity->setPos(newPos);
        entityRect = entity->getCollisionRect();
    }
}

void GameWindow::updateBar(){
    int width = (int)((double)m_player->getHealth() / (double)m_player->getMaxHealth() * 200.0);
    m_hpBar->setFixedSize(width,25);
    width = (int)((double)m_player->getMana() / (double)m_player->getMaxMana() * 200.0);
    m_manaBar->setFixedSize(width,25);
    int height = (int)((double)(m_player->getMaxSkillCoolDown() - m_player->getSkillCoolDown()) / (double)m_player->getMaxSkillCoolDown() * 60);
    m_skillBar->setFixedSize(20,height);
    return;
}

void GameWindow::updateGold(){
    QPixmap result = Item::goldIcon();
    QPainter painter(&result);
    // 绘制数字
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(22, 1, 60, 18, Qt::AlignCenter, QString::number(m_player->getGold()));
    m_goldLabel->setPixmap(result);
    return;
}

void GameWindow::worldReverse(){
    pauseGame();
    m_player->setTarget(nullptr);
    GameMap::instance()->resetCurLevel();
    QVector<Enemy*> enemies = EntityManager::instance()->getEntitiesByType<Enemy>();
    for(Enemy* e : enemies){
        e->kill();
    }
    EntityManager::instance()->destroyAllEnemies();
    transitionBase();
    return;
}

void GameWindow::transitionBase(){
    AudioManager::instance()->playSound(AudioManager::TRANSITION);
    m_transitionLabel->show();
    if(inSun){
        transPix = transitionMoon;
    }else{
        transPix = transitionSun;
    }
    inSun = !inSun;
    inMoon = !inMoon;
    int n = GAME_VIEW_HEIGHT;
    int m = GAME_VIEW_WIDTH;
    m_visit = std::vector<std::vector<bool>>(n,std::vector<bool>(m,false));
    while(!m_queue.empty()){m_queue.pop();}
    m_queue.push(QPoint(0,0));
    m_visit[0][0] = true;
    m_transitionPixmap->fill(Qt::transparent);
    m_transitionInTimer.start(15);
    return;
}

void GameWindow::transitionIn(){
    if(!m_transitionInTimer.isActive()){return;}
    int dir[][2] = {0,1,0,-1,1,0,-1,0};
    int n = GAME_VIEW_HEIGHT;
    int m = GAME_VIEW_WIDTH;
    for(int i=0;i<20 && !m_queue.empty();i++){
        int size = m_queue.size();
        while(size--){
            QPoint cur = m_queue.front();
            int cx = cur.x();
            int cy = cur.y();
            m_queue.pop();
            QPainter painter(m_transitionPixmap);
            painter.drawPixmap(cx,cy,transPix->copy(cx,cy,1,1));
            painter.end();
            for(int k=0;k<4;k++){
                int nx = cx + dir[k][0];
                int ny = cy + dir[k][1];
                if(nx >= 0 && nx < m && ny >= 0 && ny < n && !m_visit[ny][nx]){
                    m_visit[ny][nx] = true;
                    m_queue.push(QPoint(nx,ny));
                }
            }
        }
    }
    m_transitionLabel->setPixmap(*m_transitionPixmap);
    if(m_queue.empty()){
        m_transitionInTimer.stop();
        QPointF tmp = m_player->pos();
        m_player->setPosition(m_player->getAnotherPos());
        m_player->setAnotherPos(tmp);
        if(m_pet && m_pet->isAlive()){
            m_pet->QGraphicsPixmapItem::setPos(m_player->pos() + QPointF(1,1));
        }
        m_currentCameraCenter = m_player->pos();
        m_gameView->centerOn(m_currentCameraCenter);
        QTimer::singleShot(100,this,[this](){
            if(inSun){
                m_darknessItem->hide();
                AudioManager::instance()->stopSound(AudioManager::MOON_ATMOSPHERE);
            }else{
                m_darknessItem->show();
                AudioManager::instance()->playSound(AudioManager::MOON_ATMOSPHERE);
            }
            resumeGame();
            m_transitionLabel->clear();
            m_transitionLabel->hide();

        });
    }
    return;
}

void GameWindow::timeStopSkill(){
    if(m_player->getSkillCoolDown() > 0) {return;}
    pauseGame();
    AudioManager::instance()->playSound(AudioManager::TIME_STOP);
    if(inMoon){
        CAMERA_SMOOTH_FACTOR = 1;
    }
    m_player->setTarget(nullptr);
    isTimeStop = true;
    while(!m_timeStopQueue.empty()){
        m_timeStopQueue.pop();
    }
    m_timeStopQueue.push(m_player->pos());
    m_timeStopTimer.start(16);
    QTimer::singleShot(3000,this,[this](){
        m_timeStopQueue.push(m_player->pos());
        isTimeStop = false;
        m_timeStopTimer.stop();
        resumeGame();
        m_timeStopOverTimer.start(30);
        m_player->resetSkillCoolDown();
    });
    return;
}

void GameWindow::timeStopSkillOver(){
    if(m_timeStopQueue.empty()){
        CAMERA_SMOOTH_FACTOR = 0.1;
        m_timeStopOverTimer.stop();
        return;
    }
    m_player->setPosition(m_timeStopQueue.front());
    m_timeStopQueue.pop();
    m_player->timeStopAttack();
    return;
}

void GameWindow::drawLight()
{
    if(inSun){return;}
    QPoint offset = m_gameView->mapFromScene(m_player->pos()) - QPoint(GAME_VIEW_WIDTH / 2,GAME_VIEW_HEIGHT / 2);
    m_darknessItem->setPos(m_gameView->mapToScene(QPoint(-400, -400) + offset));
    return;
}

void GameWindow::drawLightBoss(){
    if(inSun){return;}
    QPoint offset = m_gameView->mapFromScene(m_currentCameraCenter) - QPoint(GAME_VIEW_WIDTH / 2,GAME_VIEW_HEIGHT / 2);
    m_darknessItem->setPos(m_gameView->mapToScene(QPoint(-400, -400) + offset));
    return;
}

void GameWindow::updateBossLevel(){
    startBossLevel();
    finishBossLevel();
    return;
}

void GameWindow::startBossLevel(){
    static int flag = 1;
    if(!GameMap::instance()->startBossLevel() || !flag){return;}
    flag--;
    m_bossTalkTimer.start(10000);
    pauseGame();
    moveCameraTarget(bossRoomCenter);
    setCameraSmoothFactor(0.02);
    m_bossRoomTimer.start(16);
    AudioManager::instance()->playSound(AudioManager::BOSS_WARNING,4);
    QTimer::singleShot(4000,this,[this](){
        moveCameraTarget(m_player);
    });
    QTimer::singleShot(8000,this,[this](){
        m_bossRoomTimer.stop();
        setCameraSmoothFactor(0.1);
        AudioManager::instance()->playSound(AudioManager::BOSS_ATMOSPHERE);
        resumeGame();
    });
    return;
}

void GameWindow::finishBossLevel(){
    static int flag = 1;
    if(!GameMap::instance()->finishBossLevel() || !flag){return;}
    flag--;
    AudioManager::instance()->stopSound(AudioManager::BOSS_ATMOSPHERE);
    m_bossTalkTimer.stop();
    return;
}

void GameWindow::createAIWeapon(){
    if(!AIManager::instance()->isEnabled()){return;}
    AIManager::instance()->requestWeapon(
        "生成一把武器",
        [this](const QString& name, int attack, const QString& type) {
            // 这是回调 A
            qDebug() <<"deepseek 返回数据成功!!!!" << name << attack << type;
            QPixmap icon;
            Weapon::WeaponType w_type;
            if(type == "弓"){
                icon = Weapon::getBowIcon();
                w_type = Weapon::BOW;
            }else{
                icon = Weapon::getSwordIcon();
                w_type = Weapon::SWORD;
            }
            Weapon* weapon = new Weapon(Item::WEAPON,name,icon,w_type,attack,attack / 10);
            m_shop->addItem(weapon,attack * 20,1);
        },
        [](const QString& error) {
            qDebug() << error;
        }
        );
    return;
}

void GameWindow::createAITalk(){
    static bool flag = true;
    if(!AIManager::instance()->isEnabled() || !flag){return;}
    QString context;
    if(GameMap::instance()->startBossLevel()){//还没打boss
        context = QString("玩家打败了你的手下,玩家剩余血量%1%%,玩家剩余魔力值%2%%").
                  arg(m_player->getHealthPercent()).arg(m_player->getManaPercent());
    }else if(GameMap::instance()->finishBossLevel()){//打好boss了
        context = QString("玩家打败你了");
        flag = false;
    }else{//正在打boss
        context = QString("玩家正在与你战斗,玩家剩余血量%1%%,玩家剩余魔力值%2%%").
                  arg(m_player->getHealthPercent()).arg(m_player->getManaPercent());
    }
    AIManager::instance()->requestBossTalk(
        context,
        [this](const QString& talk) {
            showFloatingText(m_player->pos(),talk,Qt::red);
        },
        [this](const QString& error) {
            qDebug() << error;
            showFloatingText(m_player->pos(),"网络或API-Key错误",Qt::red);
        }
        );
}

void GameWindow::showFloatingText(QPointF worldPos,QString text,QColor color){
    QGraphicsTextItem* floatingText = new QGraphicsTextItem(text);
    floatingText->setDefaultTextColor(color);
    floatingText->setFont(QFont("Arial", 20, QFont::Bold));
    floatingText->setPos(worldPos);
    floatingText->setZValue(-1);
    m_gameScene->addItem(floatingText);

    // 浮起动画
    QTimer* floatTimer = new QTimer(this);
    int step = 0;
    connect(floatTimer, &QTimer::timeout, this, [floatingText, floatTimer, step]() mutable {
        if (step < 20) {
            floatingText->setPos(floatingText->pos() + QPointF(0, -2));  // 向上飘
            floatingText->setOpacity(1.0 - step / 20.0);                  // 渐淡
            step++;
        } else {
            floatTimer->stop();
            floatingText->scene()->removeItem(floatingText);
            delete floatingText;
            floatTimer->deleteLater();
        }
    });
    floatTimer->start(250);
}
