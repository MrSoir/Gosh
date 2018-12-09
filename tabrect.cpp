#include "tabrect.h"

TabRect::TabRect(const QString &str,
                 const QFont &font,
                 const QSize &size,
                 const QPoint &pos,
                 const QColor &gradCol1,
                 const QColor &gradCol2,
                 const QColor &selectionCol1,
                 const QColor &selectionCol2,
                 const QColor &textCol1, const QColor &textCol2,
                 QGraphicsItem *parent)
    : GraphicsItemBD(size, pos, parent),
      m_gradCol1(gradCol1),
      m_gradCol2(gradCol2),
      m_selectCol1(selectionCol1),
      m_selectCol2(selectionCol2),

      m_str(str),
      m_font(font),
      m_textCol1(textCol1),
      m_textCol2(textCol2)
{
    m_closeRect = QRect(0,0,0,0);
    setAcceptHoverEvents(true);
}

TabRect::~TabRect()
{
    m_onTabClicked   = nullptr;
    m_onCloseClicked = nullptr;
}

void TabRect::setCallbackFunctions(std::function<void()> onTabClicked,
                                   std::function<void()> onCloseClicked)
{
    m_onTabClicked   = onTabClicked;
    m_onCloseClicked = onCloseClicked;
}

void TabRect::setFont(QFont font)
{
    m_font = font;
}

void TabRect::setText(QString str)
{
    m_str = str;
}

void TabRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    GraphicItemsBD::GraphicsItemBD::paint(painter, option, widget);

    QRectF rct = boundingRect();
    painter->setClipRect(rct);

    painter->setRenderHint(QPainter::Antialiasing, true);

    QColor grad1, grad2;
    if(mouIn)
    {
        grad1 = m_selectCol1;
        grad2 = m_selectCol2;
    }else
    {
        grad1 = m_gradCol1;
        grad2 = m_gradCol2;
    }

    QColor borderCol(0,0,0, 255);
    int bordWidth = 1;
    QLinearGradient gradient = QLinearGradient(rct.topLeft(), rct.bottomRight());
    gradient.setColorAt(0, grad1);
    gradient.setColorAt(1, grad2);
    painter->setBrush(QBrush(gradient));
    painter->setPen(QPen(borderCol, bordWidth));

    QPainterPath path = getBoundingPath();
    painter->setClipPath(path);
    painter->drawPath(path);

    QRectF textRect = getTextBoundingRect();
    painter->setClipRect(textRect);

    painter->setBrush(QBrush(QColor(0,0,0, 255)));
    painter->setPen(QPen(mouIn ? m_textCol2 : m_textCol1));
    painter->drawText(textRect, m_str);

    painter->setClipRect(rct);
    m_closeRect = QRect(textRect.right(),
                        rct.top(),
                        m_text_paddingXright,
                        rct.height());

    QColor crossCol = mouInClos ? m_textCol2 : m_textCol1;
    painter->setPen(QPen(crossCol));
    QPoint crossCenter( textRect.right() + m_text_paddingXright*0.5,
                        rct.center().y() );
    paintCross(crossCenter, int(m_text_paddingXright*0.8), int(rct.height()*0.5), painter);
}

void TabRect::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QPoint mouP(event->pos().x(), event->pos().y());

    if( inClose(mouP) )
    {
        if(m_onCloseClicked)
            m_onCloseClicked();
    }else
    {
        if(m_onTabClicked)
            m_onTabClicked();
    }

    revalMousePos(mouP);
    update();
}

void TabRect::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    revalMousePos(event->pos());
    update();
}

void TabRect::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    mouIn = false;
    mouInClos = false;
    update();
}

void TabRect::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    revalMousePos(event->pos());
    update();
}

void TabRect::paintCross(QPoint centerP, int width, int height, QPainter* painter)
{
    QPoint p1(centerP.x() - width*0.5, centerP.y() - height*0.5);
    QPoint p2(centerP.x() + width*0.5, centerP.y() + height*0.5);
    painter->drawLine(p1,p2);

    p1 = QPoint(centerP.x() - width*0.5, centerP.y() + height*0.5);
    p2 = QPoint(centerP.x() + width*0.5, centerP.y() - height*0.5);
    painter->drawLine(p1,p2);
}

void TabRect::revalMousePos(QPointF mouP)
{
    if( inClose(mouP) )
    {
        mouIn = false;
        mouInClos = true;
    }else
    {
        mouIn = true;
        mouInClos = false;
    }
}

QPainterPath TabRect::getBoundingPath() const
{
    QRectF rect = boundingRect();
    QPainterPath path;

    path.moveTo(rect.left(), rect.bottom());
    path.quadTo(rect.left(), rect.center().y(),
                rect.left()+m_cornerWidth, rect.top());
    path.lineTo(rect.right()-m_cornerWidth, rect.top());
    path.quadTo(rect.right(), rect.center().y(),
                rect.right(), rect.bottom());
    path.lineTo(rect.left(), rect.bottom());
    path.closeSubpath();

//    path.moveTo(rect.left(), rect.bottom());
//    path.quadTo(rect.left(), rect.top(),
//                rect.left()+m_cornerWidth, rect.top());
//    path.lineTo(rect.right()-m_cornerWidth, rect.top());
//    path.quadTo(rect.right(), rect.top(),
//                rect.right(), rect.bottom());
//    path.lineTo(rect.left(), rect.bottom());
//    path.closeSubpath();

//    path.moveTo(rect.left()+m_cornerWidth, rect.top());
//    path.lineTo(rect.left(), rect.bottom());
//    path.lineTo(rect.right(), rect.bottom());
//    path.lineTo(rect.right()-m_cornerWidth, rect.top());
//    path.closeSubpath();
    return path;
}

QRect TabRect::getTextBoundingRect() const
{
    QRectF rect = boundingRect();
    QRect textRect(rect.left() +m_cornerWidth + m_text_paddingXleft,
                   rect.top() +m_text_paddingYtop,
                   rect.width() - 2*m_cornerWidth - (m_text_paddingXleft + m_text_paddingXright),
                   rect.height() - (m_text_paddingYtop + m_text_paddingYbottom));
    return textRect;
}

bool TabRect::inClose(QPoint mouP) const
{
    return m_closeRect.contains(mouP);
}
bool TabRect::inClose(QPointF mouPf) const
{
    QPoint mouP( int(mouPf.x()), int(mouPf.y()) );
    return m_closeRect.contains(mouP);
}
