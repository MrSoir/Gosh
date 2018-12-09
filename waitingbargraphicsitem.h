#ifndef WAITINGBARGRAPHICSITEM_H
#define WAITINGBARGRAPHICSITEM_H

#include <QObject>
#include <QDebug>
#include <QString>
#include <QGraphicsItem>
#include <QRectF>
#include <QPainter>
#include <QTimer>

#include <functional>
#include <memory>
#include <math.h>

#include "customgraphicitems.h"

struct RectColor{
public:
    RectColor() :
        rectColor(minColVal, minColVal, maxColVal),
        curRctCol(minColVal, minColVal, maxColVal)
    {}

    void reset()
    {
        rectColor = QColor(minColVal, minColVal, maxColVal);
        id = 0;
        appreciating = true;

        prepareForNextRound();
    }

    void prepareForNextRound()
    {
        curRctCol = rectColor;
        curId = id;
        curAppreciating = appreciating;
    }
    void nextCurrentColor()
    {
        genNextColor(curRctCol, &curId, offs, minColVal, maxColVal, &curAppreciating);
    }
    void nextColor()
    {
        genNextColor(rectColor, &id   , offs, minColVal, maxColVal, &appreciating   );
    }

    void genNextColor(QColor& col, int* id, int offs, int minColVal, int maxColVal, bool* appreciating)
    {
        float val;
        float val_prev;

        switch(*id)
        {
        case 0:
            val = col.red();
            val_prev = col.blue();
            break;
        case 1:
            val = col.green();
            val_prev = col.red();
            break;
        case 2:
            val = col.blue();
            val_prev = col.green();
            break;
        }

        int oldId = *id;

        if( *appreciating )
        {
            val += offs;
            if(val > maxColVal)
            {
                val = maxColVal;
                *appreciating = false;
            }
        }else
        {
            val_prev -= offs;
            if(val_prev < minColVal)
            {
                val_prev = minColVal;
                *id = (*id +1) % 3;
                *appreciating = true;
            }
        }

        switch (oldId)
        {
        case 0:
            col.setRed( val );
            col.setBlue( val_prev );
            break;
        case 1:
            col.setGreen( val );
            col.setRed( val_prev );
            break;
        case 2:
            col.setBlue( val );
            col.setGreen( val_prev );
            break;
        }
    }

    const QColor getCurrentColor()
    {
        return curRctCol;
    }
private:
    float minColVal = 0;
    float maxColVal = 255;
    int curId = 0;
    int id = 0;
    int offs = 5;
    bool curAppreciating = true;
    bool appreciating = true;

    QColor rectColor;
    QColor curRctCol;
};


class WaitingBarGraphicsItem : public GraphicItemsBD::GraphicsItemBD
{
public:
    WaitingBarGraphicsItem(const QSize &size, const QPoint &pos,
                           int rectCount = 20,//m_rectCount(20),
                           int padding = 5,//m_padding(5),
                           QGraphicsItem* parent = nullptr);

    ~WaitingBarGraphicsItem();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void startAnimation();
    void stopAnimation();
    bool isRunning();

private:
    RectColor m_rectCol;
    int m_rectCount;
    int m_padding;

    QSize m_size;
    QPoint m_pos;

    QTimer* m_animationTimer;
    bool m_isRunning;
    int m_frameRate = 20;
};

#endif // WAITINGBARGRAPHICSITEM_H
