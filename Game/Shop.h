#ifndef SHOP_H
#define SHOP_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPixmap>
#include <QGraphicsItem>
#include "item.h"
class Weapon;
class Player;
class QGraphicsScene;
class QGraphicsView;
class QGraphicsPixmapItem;
class QGraphicsTextItem;
class QGraphicsRectItem;
class QPushButton;
class QGraphicsProxyWidget;
struct ShopItem {
    int id;
    Item::ItemType type;
    QString name;
    QString description;
    QPixmap icon;
    int price;
    int itemCount;
    Item* item;
    ShopItem() : id(-1), type(Item::POTION), name(""),itemCount(1),
        price(0),item(nullptr){}
};
class Shop : public QObject
{
    Q_OBJECT

public:
    explicit Shop(QObject* parent = nullptr);
    ~Shop();

    // ========== 静态参数配置 ==========
    static int getGridRows() { return GRID_ROWS; }
    static int getGridCols() { return GRID_COLS; }
    static int getCellWidth() { return CELL_WIDTH; }
    static int getCellHeight() { return CELL_HEIGHT; }

    static void setGridRows(int rows) { GRID_ROWS = rows; }
    static void setGridCols(int cols) { GRID_COLS = cols; }

    // ========== UI 图片导入接口 ==========
    void setBackgroundImage(const QString& imagePath);
    void setCellBackgroundImage(const QString& imagePath);
    void setButtonImage(const QString& imagePath);
    void setButtonHoverImage(const QString& imagePath);
    void setButtonDisabledImage(const QString& imagePath);
    void setTitleImage(const QString& imagePath);
    // ========== 商品管理接口 ==========
    void addItem(Item* item,int price,int number = 999999);
    void clearItems();
    QVector<ShopItem>& getItems() { return m_items; }

    // ========== 商店显示接口 ==========
    void show(QGraphicsScene* scene, QGraphicsView* view, int x, int y, Player* player);
    void hide();
    bool isVisible() const { return m_isVisible; }

signals:
    void gamePauseRequested();
    void gameResumeRequested();
    void itemPurchased();
    void updateItemDisplay(Item::ItemType type);

private:
    void loadIcons();
    void createUI();
    void updateDisplay();
    void createItemDisplay(int index);
    void onBuyButtonClicked(int index);
    void showMessage(const QString& msg, const QColor& color);
    // 绘制默认UI（后备方案）
    void drawDefaultBackground(int x, int y, int width, int height);
    void drawDefaultCell(int x, int y, bool isSoldOut);
    QPushButton* createDefaultButton(const QString& text, bool enabled);
    QPixmap drawDefaultIconByType(Item::ItemType type, int size);

    // ========== 静态参数 ==========
    static int GRID_ROWS;
    static int GRID_COLS;
    static int CELL_WIDTH;
    static int CELL_HEIGHT;
    static int ICON_SIZE;

    // ========== 商品数据 ==========
    QVector<ShopItem> m_items;
    QMap<int, ShopItem> m_baseItems;

    // ========== UI 图片资源 ==========
    QPixmap m_backgroundImage;
    QPixmap m_cellImage;
    QPixmap m_buttonImage;
    QPixmap m_buttonHoverImage;
    QPixmap m_buttonDisabledImage;
    QPixmap m_titleImage;

    QMap<int, QPixmap> m_itemIcons;

    bool m_hasBackgroundImage;
    bool m_hasCellImage;
    bool m_hasButtonImage;

    // ========== UI元素 ==========
    QGraphicsScene* m_scene;
    QGraphicsView* m_view;
    QGraphicsItem* m_backgroundItem;
    QGraphicsPixmapItem* m_titleItem;
    QVector<QGraphicsRectItem*> m_cellItems;     // 统一为 QGraphicsRectItem
    QVector<QGraphicsPixmapItem*> m_iconItems;
    QVector<QGraphicsTextItem*> m_nameTexts;
    QVector<QGraphicsTextItem*> m_priceTexts;
    QVector<QPushButton*> m_buyButtons;
    QVector<QGraphicsProxyWidget*> m_proxies;
    QVector<QGraphicsTextItem*> m_soldOutTexts;

    // ========== 状态 ==========
    bool m_isVisible;
    qreal m_offsetX;
    qreal m_offsetY;
    Player* m_currentPlayer;
    QGraphicsTextItem* m_titleText;  // 标题文字
};

#endif // SHOP_H
