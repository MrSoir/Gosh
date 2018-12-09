#ifndef TABRECT_H
#define TABRECT_H

#include <QObject>
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

#include <vector>
#include <memory>
#include <functional>

#include "customgraphicitems.h"

class TabRect : public GraphicItemsBD::GraphicsItemBD{
public:
    explicit TabRect(const QString& str,
                     const QFont& font,
                     const QSize& size,
                     const QPoint& pos,
                     const QColor& gradCol1,
                     const QColor& gradCol2,
                     const QColor& selectionCol1 = QColor(255,255,255),
                     const QColor& selectionCol2 = QColor(255,255,255),
                     const QColor& textCol1 = QColor(0,0,0),
                     const QColor& textCol2 = QColor(0,0,0),
                     QGraphicsItem* parent = nullptr);

    ~TabRect();

    void setCallbackFunctions(std::function<void()> onTabClicked,
                              std::function<void()> onCloseClicked);

    void setFont(QFont font);

    void setText(QString str);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void hoverMoveEvent (QGraphicsSceneHoverEvent *event);

private:
    void paintCross(QPoint centerP, int width, int height, QPainter* painter);

    void revalMousePos(QPointF mouP);

    QPainterPath getBoundingPath() const;
    QRect getTextBoundingRect() const;

    bool inClose(QPoint mouP) const;
    bool inClose(QPointF mouP) const;

    QColor m_gradCol1;
    QColor m_gradCol2;
    QColor m_selectCol1;
    QColor m_selectCol2;

    bool mouIn = false;
    bool mouInClos = false;

    int m_text_paddingYtop = 2;
    int m_text_paddingYbottom = 2;
    int m_text_paddingXleft = 0;
    int m_text_paddingXright = 10;

    int m_cornerWidth = 10;

    QRect m_closeRect;

    std::function<void()> m_onTabClicked = nullptr;
    std::function<void()> m_onCloseClicked = nullptr;

    QString m_str;
    QColor m_textCol1, m_textCol2;
    QFont m_font;
};
#endif // TABRECT_H
