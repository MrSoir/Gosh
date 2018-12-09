#ifndef TABSELECTORPANE_H
#define TABSELECTORPANE_H

#include <QObject>
#include <QString>
#include <QGraphicsView>
#include <QFileInfo>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QColor>
#include <QRect>
#include <QRectF>
#include <QPainterPath>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QFileInfo>
#include <QMouseEvent>
#include <QEvent>
#include <QTimer>

#include <vector>
#include <memory>
#include <functional>
#include <string>

#include "staticfunctions.h"
#include "customgraphicitems.h"
#include "tabrect.h"

class TabSelectorPane : public QGraphicsView
{
    Q_OBJECT
public:
    explicit TabSelectorPane(int curActiveTabId,
                             QVector<QDir> tabLabels,
                             int scrollOffsetX = 0,
                             QWidget* parent = nullptr);
    ~TabSelectorPane();

    void resizeEvent(QResizeEvent* event) override;


    int getScrollOffsetX();

//    void blockButtons(bool blockBtns);
//    bool isBlocked();
signals:
    void addClicked();
    void tabClicked(int id);
    void tabCloseClicked(int id);
public slots:
    void revalidate();
    void setTabs(QVector<QDir> tabLabels);
    void setActiveTabId(int id);
protected:
    void mouseMoveEvent(QMouseEvent *event);

    void enterEvent(QEvent *event);
    void leaveEvent(QEvent* event);
private:
    void checkForMouseScrolling(int mouseX);
    void rePaintCanvas();

    void validateScrollOffset();
    void initScrolling();
    void killScrollTimer();

    int determineUsedPaneWidth();

    void createTab(int x, int id);
    TabRect* createTabRect(QPoint pos, const int id, QString tabLabel) const;

    QVector<QDir> m_tabLabels;

//    int m_tabWidth = 400;
    int m_tabWidth = 180;
    int m_tabHeight = 20;
    int m_tabInsets = -8;

    QGraphicsScene* m_scene;

    QColor m_tabCol1 = QColor(180,180,180, 255);
    QColor m_tabCol2 = QColor(180,180,180, 255);
//    ----------------------------------------------------
    QColor m_selCol1 = QColor(255,255,255, 255);
    QColor m_selCol2 = QColor(255,200,200, 255);
//    ----------------------------------------------------
//    QColor m_activeCol1 = QColor(255,255,255, 255);
//    QColor m_activeCol2 = QColor(200,255,200, 255);
    QColor m_activeCol1 = QColor(0,0,0, 255);
    QColor m_activeCol2 = QColor(0,0,0, 255);
//    ----------------------------------------------------
    QColor m_textCol1 = QColor(255,255,255, 255);
    QColor m_textCol2 = QColor(0,0,0, 255);
    QColor m_textColActiv1 = QColor(255,255,255, 255);
    QColor m_textColActiv2 = QColor(255,150,150, 255);


    int m_paneHeight = 20;
    int m_addBtnHeight = 18;

    int m_curActiveTabId = 0;

    int m_scrollOffsetX = 0;

    QTimer* m_scrollTimer = nullptr;
    bool m_isScrollingLeft = false;
    bool m_isScrollingRight = false;
    const int m_scrollingOffset = 6;
    int m_scrollAreaWidth = 50;


//    bool m_blockButtons = false;
};

#endif // TABSELECTORPANE_H
