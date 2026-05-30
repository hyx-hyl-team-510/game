#include "Shop.h"
#include "Player.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QPushButton>
#include <QGraphicsProxyWidget>
#include <QTimer>
#include <QPainter>
#include <QDebug>
#include <QPointer>
// ========== 初始化静态参数 ==========
int Shop::GRID_ROWS = 3;
int Shop::GRID_COLS = 4;
int Shop::CELL_WIDTH = 130;
int Shop::CELL_HEIGHT = 160;
int Shop::ICON_SIZE = 48;
Shop::Shop(QObject* parent)
    : QObject(parent)
    , m_scene(nullptr)
    , m_view(nullptr)
    , m_backgroundItem(nullptr)
    , m_titleItem(nullptr)
    , m_titleText(nullptr)
    , m_isVisible(false)
    , m_offsetX(0)
    , m_offsetY(0)
    , m_currentPlayer(nullptr)
    , m_hasBackgroundImage(false)
    , m_hasCellImage(false)
    , m_hasButtonImage(false)
{
    loadIcons();
    createUI();
}

Shop::~Shop()
{
    hide();
}

// ========== UI 图片导入接口 ==========
void Shop::setBackgroundImage(const QString& imagePath)
{
    if (m_backgroundImage.load(imagePath)) {
        m_hasBackgroundImage = true;
        qDebug() << "Loaded background image:" << imagePath;
    } else {
        qDebug() << "Failed to load background image:" << imagePath;
    }
}

void Shop::setCellBackgroundImage(const QString& imagePath)
{
    if (m_cellImage.load(imagePath)) {
        m_hasCellImage = true;
        qDebug() << "Loaded cell background image:" << imagePath;
    } else {
        qDebug() << "Failed to load cell background image:" << imagePath;
    }
}

void Shop::setButtonImage(const QString& imagePath)
{
    if (m_buttonImage.load(imagePath)) {
        m_hasButtonImage = true;
        qDebug() << "Loaded button image:" << imagePath;
    } else {
        qDebug() << "Failed to load button image:" << imagePath;
    }
}

void Shop::setButtonHoverImage(const QString& imagePath)
{
    m_buttonHoverImage.load(imagePath);
}

void Shop::setButtonDisabledImage(const QString& imagePath)
{
    m_buttonDisabledImage.load(imagePath);
}

void Shop::setTitleImage(const QString& imagePath)
{
    m_titleImage.load(imagePath);
}

void Shop::loadIcons()
{
    // 预设商品图标可以通过此函数加载
}

void Shop::drawDefaultBackground(int x, int y, int width, int height)
{
    QGraphicsRectItem* bg = new QGraphicsRectItem(x, y, width, height);
    bg->setBrush(QBrush(QColor(30, 30, 50, 240)));
    bg->setPen(QPen(Qt::white, 2));
    m_scene->addItem(bg);
    m_backgroundItem = bg;
}

void Shop::drawDefaultCell(int x, int y, bool isSoldOut)
{
    QGraphicsRectItem* cell = new QGraphicsRectItem(x, y, CELL_WIDTH, CELL_HEIGHT);
    cell->setBrush(isSoldOut ? QBrush(QColor(60, 60, 70)) : QBrush(QColor(50, 50, 70)));
    cell->setPen(QPen(Qt::gray, 1));
    m_scene->addItem(cell);
    m_cellItems.append(cell);
}

QPushButton* Shop::createDefaultButton(const QString& text, bool enabled)
{
    QPushButton* btn = new QPushButton(text);
    btn->setFixedSize(70, 28);
    if (enabled) {
        btn->setStyleSheet(
            "QPushButton { background-color: #4CAF50; color: white; border-radius: 4px; font-weight: bold; }"
            "QPushButton:hover { background-color: #45a049; }"
            "QPushButton:pressed { background-color: #3d8b40; }"
            );
    } else {
        btn->setStyleSheet(
            "QPushButton { background-color: #555555; color: #888888; border-radius: 4px; font-weight: bold; }"
            );
    }
    btn->setEnabled(enabled);
    return btn;
}

void Shop::createUI()
{
    int maxItems = GRID_ROWS * GRID_COLS;
    for (int i = 0; i < maxItems; i++) {
        m_cellItems.append(nullptr);
        m_iconItems.append(nullptr);
        m_nameTexts.append(nullptr);
        m_priceTexts.append(nullptr);
        m_buyButtons.append(nullptr);
        m_proxies.append(nullptr);
        m_soldOutTexts.append(nullptr);
    }
}

void Shop::showMessage(const QString& msg, const QColor& color)
{
    if (!m_scene) return;

    QGraphicsTextItem* tip = new QGraphicsTextItem(msg);
    tip->setDefaultTextColor(color);
    tip->setFont(QFont("Arial", 14, QFont::Bold));
    tip->setPos(m_offsetX + 50, m_offsetY - 35);
    m_scene->addItem(tip);
    QPointer<QGraphicsTextItem> self(tip);
    QTimer::singleShot(1500, [self]() {
        if (!self.isNull() && self->scene()) {
            self->scene()->removeItem(self);
            delete self;
        }
    });
}

void Shop::createItemDisplay(int index)
{
    if (index >= m_items.size()) return;

    const ShopItem& item = m_items[index];
    bool isSoldOut = item.itemCount > 0 ? false : true;
    int row = index / GRID_COLS;
    int col = index % GRID_COLS;
    int x = m_offsetX + col * CELL_WIDTH;
    int y = m_offsetY + row * CELL_HEIGHT;

    // 1. 单元格背景（仅在设置了自定义图片时显示）
    QGraphicsRectItem* cell = new QGraphicsRectItem(x, y, CELL_WIDTH, CELL_HEIGHT);
    if (m_hasCellImage) {
        QBrush brush(m_cellImage.scaled(CELL_WIDTH, CELL_HEIGHT));
        cell->setBrush(brush);
        cell->setPen(QPen(Qt::NoPen));
    } else {
        // 无自定义图片时完全透明
        cell->setBrush(QBrush(Qt::transparent));
        cell->setPen(QPen(Qt::NoPen));
    }
    m_scene->addItem(cell);
    m_cellItems[index] = cell;

    // 2. 商品图标
    QPixmap iconPixmap;
    if (!item.icon.isNull()) {
        iconPixmap = item.icon;
    } else {
    }

    if (!iconPixmap.isNull()) {
        if (isSoldOut) {
            QPixmap grayIcon(ICON_SIZE, ICON_SIZE);
            grayIcon.fill(Qt::transparent);
            QPainter painter(&grayIcon);
            painter.setOpacity(0.5);
            painter.drawPixmap(0, 0, iconPixmap.scaled(ICON_SIZE, ICON_SIZE));
            painter.end();
            iconPixmap = grayIcon;
        }
        int iconX = x + (CELL_WIDTH - ICON_SIZE) / 2;
        int iconY = y + 10;
        QGraphicsPixmapItem* iconItem = new QGraphicsPixmapItem(iconPixmap.scaled(ICON_SIZE, ICON_SIZE));
        iconItem->setPos(iconX, iconY);
        m_scene->addItem(iconItem);
        m_iconItems[index] = iconItem;
    }

    // 3. 商品名称
    QGraphicsTextItem* nameText = new QGraphicsTextItem(item.name);
    nameText->setDefaultTextColor(isSoldOut ? Qt::gray : Qt::white);
    nameText->setFont(QFont("Arial", 10, QFont::Bold));
    int textWidth = nameText->boundingRect().width();
    nameText->setPos(x + (CELL_WIDTH - textWidth) / 2, y + 65);
    m_scene->addItem(nameText);
    m_nameTexts[index] = nameText;

    if (isSoldOut) {
        QGraphicsTextItem* soldOutText = new QGraphicsTextItem("已售罄");
        soldOutText->setDefaultTextColor(QColor(255, 100, 100));
        soldOutText->setFont(QFont("Arial", 12, QFont::Bold));
        textWidth = soldOutText->boundingRect().width();
        soldOutText->setPos(x + (CELL_WIDTH - textWidth) / 2, y + 90);
        m_scene->addItem(soldOutText);
        m_soldOutTexts[index] = soldOutText;

        QGraphicsTextItem* priceText = new QGraphicsTextItem("---");
        priceText->setDefaultTextColor(Qt::gray);
        priceText->setFont(QFont("Arial", 12, QFont::Bold));
        textWidth = priceText->boundingRect().width();
        priceText->setPos(x + (CELL_WIDTH - textWidth) / 2, y + 115);
        m_scene->addItem(priceText);
        m_priceTexts[index] = priceText;

        QPushButton* buyBtn = createDefaultButton("已售罄", false);
        QGraphicsProxyWidget* proxy = m_scene->addWidget(buyBtn);
        proxy->setPos(x + (CELL_WIDTH - 70) / 2, y + 120);
        m_proxies[index] = proxy;
        m_buyButtons[index] = buyBtn;
    } else {
        QString priceStr = QString("%1 G").arg(item.price);
        QGraphicsTextItem* priceText = new QGraphicsTextItem(priceStr);
        priceText->setDefaultTextColor(QColor(255, 200, 0));
        priceText->setFont(QFont("Arial", 12, QFont::Bold));
        textWidth = priceText->boundingRect().width();
        priceText->setPos(x + (CELL_WIDTH - textWidth) / 2, y + 90);
        m_scene->addItem(priceText);
        m_priceTexts[index] = priceText;

        QPushButton* buyBtn = createDefaultButton("购买", true);
        connect(buyBtn, &QPushButton::clicked, this, [this, index]() {
            onBuyButtonClicked(index);
        });

        QGraphicsProxyWidget* proxy = m_scene->addWidget(buyBtn);
        proxy->setPos(x + (CELL_WIDTH - 70) / 2, y + 120);
        m_proxies[index] = proxy;
        m_buyButtons[index] = buyBtn;
    }
}

void Shop::updateDisplay()
{
    // 清除所有旧显示
    for (int i = 0; i < m_cellItems.size(); i++) {
        if (m_cellItems[i]) {
            if (m_cellItems[i]->scene()) m_scene->removeItem(m_cellItems[i]);
            delete m_cellItems[i];
        }
        if (m_iconItems[i]) {
            if (m_iconItems[i]->scene()) m_scene->removeItem(m_iconItems[i]);
            delete m_iconItems[i];
        }
        if (m_nameTexts[i]) {
            if (m_nameTexts[i]->scene()) m_scene->removeItem(m_nameTexts[i]);
            delete m_nameTexts[i];
        }
        if (m_priceTexts[i]) {
            if (m_priceTexts[i]->scene()) m_scene->removeItem(m_priceTexts[i]);
            delete m_priceTexts[i];
        }
        if (m_proxies[i]) {
            if (m_proxies[i]->scene()) m_scene->removeItem(m_proxies[i]);
            delete m_proxies[i];
        }
        if (m_soldOutTexts[i]) {
            if (m_soldOutTexts[i]->scene()) m_scene->removeItem(m_soldOutTexts[i]);
            delete m_soldOutTexts[i];
        }
    }

    // 清空容器
    m_cellItems.clear();
    m_iconItems.clear();
    m_nameTexts.clear();
    m_priceTexts.clear();
    m_proxies.clear();
    m_soldOutTexts.clear();
    m_buyButtons.clear();

    // 重新分配大小
    int maxItems = GRID_ROWS * GRID_COLS;
    for (int i = 0; i < maxItems; i++) {
        m_cellItems.append(nullptr);
        m_iconItems.append(nullptr);
        m_nameTexts.append(nullptr);
        m_priceTexts.append(nullptr);
        m_buyButtons.append(nullptr);
        m_proxies.append(nullptr);
        m_soldOutTexts.append(nullptr);
    }

    // 创建新显示
    int displayCount = qMin(m_items.size(), GRID_ROWS * GRID_COLS);
    for (int i = 0; i < displayCount; i++) {
        createItemDisplay(i);
    }
}

void Shop::addItem(Item* item,int price,int number){
    ShopItem newitem;
    newitem.name = item->Item::getName();
    newitem.icon = item->Item::getIcon();
    newitem.price = price;
    newitem.description = "666";
    newitem.itemCount = number;
    newitem.item = item;
    newitem.type = item->Item::getType();
    newitem.id = m_items.size();
    m_items.append(newitem);
    return;
}

void Shop::onBuyButtonClicked(int index)
{
    if (index >= m_items.size()) return;
    if (m_items[index].itemCount <= 0) {
        showMessage("该商品已售罄！", QColor(255, 150, 150));
        return;
    }
    if (!m_currentPlayer) return;

    ShopItem& item = m_items[index];

    if (m_currentPlayer->getGold() < item.price) {
        showMessage(QString("金币不足！需要 %1 G").arg(item.price), QColor(255, 150, 150));
        return;
    }

    m_currentPlayer->addGold(-item.price);

    switch (item.type) {
    case Item::POTION:
        item.item->onPurchased(m_currentPlayer);
        qDebug() << "成功购买";
        item.itemCount--;
        emit updateItemDisplay(Item::POTION);
        break;
    case Item::THROWABLE:
        item.item->onPurchased(m_currentPlayer);
        qDebug() << "成功购买";
        item.itemCount--;
        emit updateItemDisplay(Item::THROWABLE);
        break;
    case Item::WEAPON:
    {
        // 检查背包容量
        if (m_currentPlayer->getCurrentWeapon() && m_currentPlayer->isWeaponInventoryFull()) {
            m_currentPlayer->addGold(item.price);
            showMessage("武器背包已满！", QColor(255, 150, 150));
            return;
        }
        item.item->onPurchased(m_currentPlayer);
        item.itemCount--;
        break;
    }
    default:
        break;
    }

    qDebug() << "Player bought:" << item.name << "for" << item.price;
    qDebug() << "itemCount = " << item.itemCount;

    emit itemPurchased();
}

void Shop::show(QGraphicsScene* scene, QGraphicsView* view, int screenX, int screenY, Player* player)
{
    if (m_isVisible) return;
    m_scene = scene;
    m_view = view;
    m_currentPlayer = player;

    QPointF scenePos = view->mapToScene(screenX, screenY);
    m_offsetX = scenePos.x();
    m_offsetY = scenePos.y();

    int totalWidth = GRID_COLS * CELL_WIDTH;
    int totalHeight = GRID_ROWS * CELL_HEIGHT;

    // 创建背景
    // 创建背景
    if (m_hasBackgroundImage) {
        QPixmap scaledBg = m_backgroundImage.scaled(totalWidth, totalHeight,
                                                    Qt::IgnoreAspectRatio,
                                                    Qt::SmoothTransformation);
        QGraphicsPixmapItem* bg = new QGraphicsPixmapItem(scaledBg);
        bg->setPos(m_offsetX, m_offsetY);
        m_scene->addItem(bg);
        m_backgroundItem = bg;  // 需要将 m_backgroundItem 的类型改为 QGraphicsItem* 或 QGraphicsPixmapItem*
    } else {
        drawDefaultBackground(m_offsetX, m_offsetY, totalWidth, totalHeight);
    }

    // 创建标题（只创建一次）
    if (!m_titleImage.isNull()) {
        m_titleItem = new QGraphicsPixmapItem(m_titleImage);
        int titleX = m_offsetX + (totalWidth - m_titleImage.width()) / 2;
        m_titleItem->setPos(titleX, m_offsetY - m_titleImage.height() - 5);
        m_scene->addItem(m_titleItem);
    } else {
        m_titleText = new QGraphicsTextItem("商店");
        m_titleText->setDefaultTextColor(QColor(255, 215, 0));
        m_titleText->setFont(QFont("Arial", 20, QFont::Bold));
        int textWidth = m_titleText->boundingRect().width();
        m_titleText->setPos(m_offsetX + (totalWidth - textWidth) / 2, m_offsetY - 35);
        m_scene->addItem(m_titleText);
    }

    updateDisplay();
    m_isVisible = true;

    emit gamePauseRequested();
}

void Shop::hide()
{
    if (!m_isVisible) return;
    if (m_backgroundItem) {
        if (m_backgroundItem->scene()) m_scene->removeItem(m_backgroundItem);
        delete m_backgroundItem;
        m_backgroundItem = nullptr;
    }
    if (m_titleItem) {
        if (m_titleItem->scene()) m_scene->removeItem(m_titleItem);
        delete m_titleItem;
        m_titleItem = nullptr;
    }
    // 清除标题文字（新增）
    if (m_titleText) {
        if (m_titleText->scene()) m_scene->removeItem(m_titleText);
        delete m_titleText;
        m_titleText = nullptr;
    }
    for (int i = 0; i < m_cellItems.size(); i++) {
        if (m_cellItems[i]) {
            if (m_cellItems[i]->scene()) m_scene->removeItem(m_cellItems[i]);
            delete m_cellItems[i];
        }
        if (m_iconItems[i]) {
            if (m_iconItems[i]->scene()) m_scene->removeItem(m_iconItems[i]);
            delete m_iconItems[i];
        }
        if (m_nameTexts[i]) {
            if (m_nameTexts[i]->scene()) m_scene->removeItem(m_nameTexts[i]);
            delete m_nameTexts[i];
        }
        if (m_priceTexts[i]) {
            if (m_priceTexts[i]->scene()) m_scene->removeItem(m_priceTexts[i]);
            delete m_priceTexts[i];
        }
        if (m_proxies[i]) {
            if (m_proxies[i]->scene()) m_scene->removeItem(m_proxies[i]);
            delete m_proxies[i];
        }
        if (m_soldOutTexts[i]) {
            if (m_soldOutTexts[i]->scene()) m_scene->removeItem(m_soldOutTexts[i]);
            delete m_soldOutTexts[i];
        }
    }

    m_cellItems.clear();
    m_iconItems.clear();
    m_nameTexts.clear();
    m_priceTexts.clear();
    m_proxies.clear();
    m_soldOutTexts.clear();
    m_buyButtons.clear();

    m_isVisible = false;

    emit gameResumeRequested();
}

void Shop::clearItems()
{
    m_items.clear();
}
